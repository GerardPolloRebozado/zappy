# Zappy AI Development & Training Documentation

**Algorithm:** Proximal Policy Optimization (PPO) via Stable-Baselines3  
**Environment:** `ZappyEnv` (Custom Gymnasium Environment using in-memory direct FFI)

---

## 1. Introduction and Project Architecture

The goal of this project is to train a Neural Network using Reinforcement Learning (RL) to autonomously survive, coordinate, and level up to Level 8 in the **Zappy** game.

To maximize execution speeds, the Python environment interacts directly with the Rust server engine compiled as a shared library (`libzappy_engine.so` / `.dylib`) rather than communicating over TCP sockets. 

```
┌────────────────────────────────────────────────────────┐
│                      Python Process                    │
│                                                        │
│   ┌──────────────┐     FFI (ctypes)     ┌──────────┐   │
│   │  ZappyEnv    │ ───────────────────> │ ZappyLib │   │
│   └──────────────┘                      └──────────┘   │
│          │                                    │        │
│          ▼ (Observation / Step)               ▼        │
│   ┌──────────────┐                      ┌──────────┐   │
│   │ Stable-B3    │                      │ C/Rust   │   │
│   │ PPO Agent    │                      │ Engine   │   │
│   └──────────────┘                      └──────────┘   │
└────────────────────────────────────────────────────────┘
```

This direct FFI bridge completely eliminates TCP serialization overhead, network stack latency, and socket port management issues, allowing the environment to process hundreds of ticks per second in a single thread.

---

## 2. Core Modules & Components

The AI training codebase is organized modularly to isolate concerns:

- **[ZappyEnv.py](file:///Users/anapallares/epi/tek2/zappy/ai/training/training_env/ZappyEnv.py)**: The main Gymnasium environment class. It handles state transitions, environment resetting, and converts game observations into the 657-dimensional neural network input vector.
- **[actions.py](file:///Users/anapallares/epi/tek2/zappy/ai/training/training_env/actions.py)**: Maps the agent's 24 discrete action outputs (movement, look, inventory, take/set items, incantation, broadcast) to FFI library commands.
- **[broadcast.py](file:///Users/anapallares/epi/tek2/zappy/ai/training/training_env/broadcast.py)**: Manages team messaging, serialization, and decryption of broadcasts.
- **[env_modes.py](file:///Users/anapallares/epi/tek2/zappy/ai/training/training_env/env_modes.py)**: Implements `LibZappyEnv`, which handles the lifecycle of the C/Rust server memory pointer (`zappy_init`, `zappy_free`, `zappy_tick`).
- **[lib_client.py](file:///Users/anapallares/epi/tek2/zappy/ai/src/client/lib_client.py)**: The ctypes FFI client wrapper that handles command transmission and response polling.

---

## 3. Resolving Training Hacks & Local Optima

During early training phases, the agent discovered reward-hacking loops instead of learning to level up:

### The "Drop and Pickup" Local Optimum Loop
- **Problem**: When pick up rewards were positive (`+10.0`) and dropping had no penalty (`0.0`), the agent would stand on a tile, drop a stone, pick it back up, and repeat this loop forever to farm infinite rewards without moving.
- **Solution (Symmetric Rewards)**: We implemented a symmetric reward system. Dropping an item now carries the exact same negative penalty as picking it up. E.g., `TAKE_RESOURCE = +10.0` and `SET_RESOURCE = -10.0`. This nets `0` points for loops, forcing the agent to explore and elevate.

### Hunger/Survival Scaling
- **Problem**: The agent would ignore food search until it starved, or spend too much time eating without trying to gather stones.
- **Solution**: We tie the reward weights dynamically to the agent's hunger status. When food is abundant, resource gathering is rewarded highly. When food levels drop towards critical limits (e.g., `< 10` food), food acquisition rewards scale up exponentially, forcing the agent to prioritize survival.

---

## 4. Continuous Training (Resuming Progress)

Training can be resumed over previously saved weights instead of starting from scratch:
- **CLI Argument**: Passing `--load-model <model_path>` loads a `.zip` weight file.
- **Usage**:
  ```bash
  ./ai/training/run_training.sh --load-model zappy_ai_model --timesteps 100000
  ```

---

## 5. Strict Headless Evaluation

To evaluate model performance objectively without launching a visual client, we use:
- **[evaluate_ai.py](file:///Users/anapallares/epi/tek2/zappy/ai/training/training_env/evaluate_ai.py)**: Runs a fast headless simulation with **2 teams of 2 players each** (4 concurrent agents controlled by the model).
- **Console & Markdown Reports**: Saves metrics (survived turns, max level reached) to `ai/training/results/`.
- **Performance Rating Tiers**:
  -  **Tier 1: Starvation/Survival Failure** (Starvation before Level 2, <2000 turns)
  -  **Tier 2: Single-Player Competence** (Reaching Level 2)
  -  **Tier 3: Early Coordination** (Reaching Level 3-4)
  -  **Tier 4: Advanced Teamwork** (Reaching Level 5-6)
  -  **Tier 5: Master Ascension** (Reaching Level 7)
  -  **Tier 6: Celestial Zenith** (Level 8 Victory)

---

## 6. Key FFI Debugging Fixes

Several critical runtime desynchronization and hang issues were resolved during development:

### Client Desynchronization (Asynchronous Elevation Alerts)
During incantations, the server broadcasts elevation statuses to all players on a tile. Non-initiating agents were receiving these messages in their response queues, throwing off their sequential command/response synchronization. We implemented `is_initiator` tracking in `ZappyLibClient` to allow non-initiating agents to correctly discard asynchronous elevation messages without corrupting their queues.

### The FFI Player Deletion Hang
When a player starved to death, the Rust engine deleted the player ID. Subsequent Python FFI command calls to `zappy_get_response` for that player ID would return `NULL` forever. This caused:
1. `wait_for_response()` to loop `max_ticks = 100000` times calling `_tick(1)` on every iteration, advancing the simulation clock by 100,000 milliseconds.
2. Alive players instantly starved to death due to the massive advanced ticks.
3. The python loop never set the client's `is_dead` to `True` on timeout, resulting in an endless, CPU-heavy command transmission loop.

**Fix**:
1. Added an early-return check to immediately return `"dead"` if the client is flagged as dead.
2. Reduced `max_ticks` from `100000` to `2000` (which is more than enough for the slowest action, `Incantation`).
3. Set `self.is_dead = True` and return `"dead"` on timeout, stopping further commands.