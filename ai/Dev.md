# AI Training and Evolution Report for Zappy (PPO)

**Algorithm:** Proximal Policy Optimization (PPO) via Stable-Baselines3  
**Environment:** `ZappyEnv` (Custom Gymnasium Environment) connected to Rust Server  

---

## 1. Introduction and Project Context

The goal of this project is to train a Neural Network using Reinforcement Learning (RL) to autonomously control a player in the **Zappy** universe. 
To achieve this, we used **Gymnasium** (`ZappyEnv`). The chosen algorithm was **PPO (Proximal Policy Optimization)** with a Multi-Layer Perceptron policy architecture (`MlpPolicy`), which is ideal for  making decisions in our game space.

---

## 2. Analysis of Training Sessions

Throughout the development, multiple executions of the `make train` command were used. After testing our AI's "brain" went through:

### Phase 1: The Initial Smoke Test (8,192 Timesteps)
In our first contact with the original controller, we obtained the following results in the terminal:
* **`ep_len_mean` (2.91e+03 = 2,910 turns):** The AI survived an average of almost 3,000 actions.
* **`ep_rew_mean` (3.46e+03 = 3,460 points):** Consistently positive scoring.
* **`entropy_loss` (-2.82):** A high entropy reflecting chaotic, exploratory, and healthy behavior. The AI was pressing buttons freely and unpredictably to discover the map.

### Phase 2: The "Broken Controller" Mirage (51,200 Timesteps)
After some modifications to the base reward system, we started a short training session and observed a better performance that looked more close to the expected future inhabitant:
* **`ep_len_mean` (1.74e+04 = 17,400 turns):** Survival improved at surprising levels.
* **`ep_rew_mean` (1.41e+05 = 141,000 points):** Total accumulated score incremented massively.
* **`entropy_loss` (-0.134 to -0.047):** An absolute collapse in entropy. The AI stopped doubting and began acting with blind robotic certainty.

#### **Technical Diagnosis:**
This performance was a **mathematical mirage**. Due to a bug in the action `IntEnum`, the neural network tried to press consecutive button numbers (0, 2, 4...) that did not logically exist in our code. Python intercepted these ghost buttons by throwing a `ValueError`, punished the AI with `-0.5` points, but **sent nothing to the network server**.

Because these erroneous turns were processed in `0.00001` seconds (in the AI's mind, without physical network delays), the AI discovered a **Reward Hack**: it learned to avoid the invisible buttons and dedicated itself to infinitely spamming free basic commands that gave fixed points (`FORWARD`, `LEFT`, `LOOK`, or `INVENTORY`), combined with the automatic collection of "food" that kept it alive. The AI wasn't intelligent; it was a lazy worker exploiting a loophole in the rules.

### Phase 3: Back to Reality (51,200 Timesteps with Fixed Action Space)
After rebuilding the control panel to **24 actions** (from 0 to 23), injecting dynamic item take/set, and fixing network crashes, the table brought us back to the hard reality of the actual game:
* **`ep_len_mean` (1.50e+04 = 15,000 turns):** A slight decrease in lifespan.
* **`ep_rew_mean` (1.09e+05 = 109,000 points):** Total score dropped by 22%.
* **`entropy_loss` (-0.0339):** The AI was still very confident in its closed action loop.

#### **Technical Diagnosis:**
The drop in score was, contrary to instinct, **excellent news**. By fixing the controller, the game became drastically harder. The AI went from having rewards from every 19 buttons to 24 real buttons (including 7 types of `TAKE`, 7 of `SET`, and the `INCANTATION` command). Now, pressing a useless button send traffic to the Rust server, consumes real game ticks and penalizes lifespan. The AI had to start learning in real conditions without assistance.

### Phase 4: Millennium Prize Problems  (1,001,472 Timesteps)
We decided to start a massive training session lasting over 36 real minutes of CPU processing:
* **`fps` (453 actions/second):** An ultra-fast computation rate thanks to socket and server manager optimization.
* **`ep_len_mean` (2.67e+04 = 26,700 turns):** The best survival time. The AI is practically immortal against starvation.
* **`ep_rew_mean` (1.31e+06 = 1,310,000 points):** We broke the one-million-point barrier per life.
* **`entropy_loss` (-0.000338):** **Absolute zero entropy**. The AI has completely eliminated any trace of randomness or exploration from its brain.

#### **Technical Diagnosis:**
Such a low entropy value (`-0.0003`) coupled with a reward of over a million points confirms that the AI has discovered a **Closed Local Optimum**. Because there is a very juicy reward for collecting stones (`TAKE = +10.0`) and a negligible penalty for dropping them (`SET = +0.0` or less), the AI has learned a dirty trick: it stands on a tile with resources, drops an item on the ground, and picks it up again in an infinite compulsive loop. It only stops when its food level drops dangerously close to 15, at which point it grabs a piece of food and in starts another time its stone loop.

---

## 3. Changelog and Code Architecture Implementation

To reach this level of stability, the following changes were made to the software:

### A. Action Space Redesign and Dynamic Mapping
We removed the rigid dependency on an unordered `IntEnum` that caused gaps and incompatibilities with PPO's linear tensors. We adopted a logical enumeration (standard Python `Enum`) and expanded the space to `spaces.Discrete(24)`.

To avoid infinite `if/elif` control structures,we used `match/case`:

```python
match action:
    case 0: response = self.client.forward(); zappy_action = ZappyAction.FORWARD
    # ... movement and vision commands from 1 to 8 ...
    
    # (TAKE)
    case _ if 9 <= action <= 15:
        item_target = ZAPPY_ITEMS[action - 9]
        response = self.client.take(item_target)
        zappy_action = ZappyAction.TAKE
        
    # (SET)
    case _ if 16 <= action <= 22:
        item_target = ZAPPY_ITEMS[action - 16]
        response = self.client.set(item_target)
        zappy_action = ZappyAction.SET
        
    case 23:
        response = self.client.incantation()
        zappy_action = ZappyAction.INCANTATION
```

### B. Critical Patch for the `Incantation` Command
An infinite loop was detected caused by the server during elevation rituals. When the AI executes an `Incantation`, the server first responds with `Elevation underway`, freezes the player for 300 real-time ticks, and finally responds with `Current level: K`.

Our original `wait_for_response()` function captured the level text, executed a `continue`, and got stuck blocking the socket waiting for data that would never arrive. We modified the function to return the text string immediately, allowing `ZappyEnv` to process the ritual's success:

```python
case s if s.startswith("Current level:"):
    try:
        self.level = int(s.split(":")[1].strip())
        logger.info(f"Event: {s}")
    except (ValueError, IndexError):
        logger.warning(f"Failed to parse level up message: {s}")
    return s
```
### C. Zombie Socket Management and Port Synchronization
During lots of restarts in training sessions, Rust server processes were left floating in the OS, changing the default network port (`4242`). When the `get_free_port()` manager detected the problem, it changed the server port (to `54321`), but the `ZappyEnv` environment kept trying to connect its network client to the old port, resulting in a collapse via `BrokenPipeError`.

Two things we implemented to solve this:
1. Sync the port in the environment's `reset()` method: `self.port = self.server_manager.port`.
2. Forcing a socket disconnection before creating a new server thread:
```python
if self.client:
    self.client.close()
```

### E. The Frequency Parameter (`-f`)
The training speed is directly tied to the Zappy server's frequency parameter:
* **Low Frequency (`-f 100` or lower):** The AI takes almost 1 real minute to consume its initial life resources. Processing 2,048 actions meant waiting entire minutes in the terminal before seeing a single PPO statistics table.
* **Extreme Frequency (`-f 100000`):** The server ran so fast that the AI's entire lifecycle was consumed in **0.012 real seconds**. Python didn't have time to read the "WELCOME" from the socket before the AI had already starved to death, causing an immediate connection error.
* **Optimized Frequency (`-f 1000` to `2000`):** The best frequency we found. It provides a good perfomance without crashing our program, pushing the training frames per second to more than **450 fps**.

---

## 4. Conclusions and Next Steps in RL Engineering

The project is currently working and our inhabitant is living its live, but its not perfect.

However, the AI's current brain suffers the **Local Optimum** problem, due to "amnesia": the `MlpPolicy` only processes the current turn, making it impossible for the AI to see medium-term strategic plans (such as intentionally walking towards a stone it saw 3 tiles away).

To transform this simple AI into better one capable of reach Level 8, we should implement:

1. **Implement Frame Stacking (Short-Term Memory):** Wrap the environment using the Gymnasium `FrameStack` wrapper. This will concatenate the last 4 observations (the last 4 `LOOK` and `INVENTORY` vectors), allowing the neural network to process the *direction, speed, and persistence* of objects in space, granting it short-term memory to navigate the map effectively.
2. **Dynamic Entropy Injection (`ent_coef`):** Maintain a controlled entropy coefficient in `train.py` (`ent_coef=0.01` or `0.02`) during prolonged training sessions to ensure that, even if the AI accumulates millions of points, it never stops exploring alternative button combinations.