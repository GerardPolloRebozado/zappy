# AI

This is the AI project for the Multis - Zappy repository.

## Project Structure

The source code is organized into a layered architecture:

- **`src/network/`**: The communication layer. Handles TCP sockets and ensures reliable line-by-line data exchange with the server.
- **`src/client/`**: The protocol layer. Defines the `ZappyAiClient` class and maps high-level actions to the specific command strings expected by the server.
- **`src/strategy/`**: The logic layer. This is where the AI's "brain" resides, containing the main loop and decision making algorithms.
- **`src/utils/`**: Utility functions and classes that support the other layers.
- **`src/main.py`**: The entry point. Manages command-line argument parsing and initializes the client connection.
- **`training/training_env/`**: Contains the reinforcement learning environment wrappers.
  - `ZappyEnv.py`: Gymnasium environment definition and observation generation.
  - `actions.py`: Discrete action space definition.
  - `broadcast.py`: Team communications parser and heuristic calculations.
  - `env_modes.py`: Network (TCP socket) and Direct-Library (FFI ctypes) execution modes.

## Compile
### Prepare the build directory (recomended)
```bash
mkdir -p build && cd build
```
### Configure and compile
If inside build/
```bash
cmake ..
make
```
Else, from the root
```bash
cmake
make
```

### Run client
You can run ./zappy_ai from the root directory or ./build/zappy_ai
```bash
python3 client.py -p [port] -n [team_name] -ip [ip address]
```

### Manual Mode
You can start the client in manual interactive mode using the `-m` or `--manual` flag:
```bash
python3 client.py -p [port] -n [team_name] -ip [ip address] -m
```
In manual mode, you can control the AI manually by entering commands directly into the terminal prompt. You can input either the command name (case-insensitively) or its corresponding number.

#### Manual Commands Map
| Number | Command | Usage | Description |
| :---: | :--- | :--- | :--- |
| **1** | `Forward` | `1` or `forward` | Move forward one tile |
| **2** | `Right` | `2` or `right` | Turn 90 degrees right |
| **3** | `Left` | `3` or `left` | Turn 90 degrees left |
| **4** | `Look` | `4` or `look` | Look around the environment |
| **5** | `Inventory` | `5` or `inventory` | Inspect current inventory |
| **6** | `Broadcast` | `6 <text>` or `broadcast <text>` | Broadcast a message to other players |
| **7** | `Connect_nbr` | `7` or `connect_nbr` | Get number of unused connections for the team |
| **8** | `Fork` | `8` or `fork` | Fork a player connection (lay an egg) |
| **9** | `Eject` | `9` or `eject` | Eject players from this tile |
| **10** | `Take` | `10 <object>` or `take <object>` | Take/pick up an object |
| **11** | `Set` | `11 <object>` or `set <object>` | Set/drop an object |
| **12** | `Incantation` | `12` or `incantation` | Start the level elevation incantation |

If you enter a parameterized command like `broadcast`, `take`, or `set` without an argument, the interactive prompt will ask you to input the argument.

#### Special Shell Commands
- `help` or `h`: Show the manual mode command menu.
- `exit`, `quit`, or `q`: Terminate connection and exit manual mode.

## Reinforcement Learning (AI Training)

The project includes Machine Learning using **Stable-Baselines3 (PPO)** and **Gymnasium** to train a neural network bot. 

### The Training Environment (`ZappyEnv`)
The AI does not interact with the game via strings or visuals; it interacts through mathematical tensors translated by the custom `ZappyEnv`.

- **Observation Space (What the AI senses):** 657 values of a NumPy array representing the current state of the player.
  - Indices `0-6`: The player's current inventory (food, linemate, deraumere, etc.).
  - Indices `7-654`: The parsed "vision" grid (the result of the `Look` command mapped to integer counters per tile).
  - Index `655`: The player's current level.
- **Action Space (What the AI does):** 24 actions, mapped to Zappy commands via an `IntEnum`. 

### The Headless FFI Library Mode
We significantly improved training speeds by converting the Rust `zappy_server` into a shared library (`zappy_engine`). Instead of running a background server process and communicating via TCP sockets, the Python `LibZappyEnv` uses `ctypes` to invoke engine ticks and functions directly in memory. This eliminates network overhead and allows training to process hundreds of times faster.

### The Rewards System
The Reinforcement Learning agent learns optimal behavior by maximizing its cumulative score. The reward function is shaped as follows to prevent loops and optimize performance:

- **Symmetric Take & Set Rewards**:
  - Picking up a stone needed for elevation gives **`+4.0` points**; an excess stone gives **`-0.2` points**.
  - Dropping a stone that causes deficiency gives **`-4.0` points**; an excess stone gives **`+0.2` points**.
  - This symmetry makes standard drop-and-pickup loop exploits net **`0.0` points**, preventing reward-hacking.
- **Hunger-Sensitive Food Collection**:
  - Taking food when hungry (`food < 15`) gives **`+2.0` points** (high priority).
  - Taking food when satisfied (`food >= 15`) gives **`+0.2` points** (buffer maintenance).
  - Setting (dropping) food is penalized with **`-2.0` points**.
- **Scaled Elevation Success**:
  - Reaching target level `K` awards a scaled reward of **`+100.0 * K` points** (e.g. `+200.0` for Level 2, `+800.0` for Level 8).
- **Failed Incantation Penalty**:
  - Trying to incant but getting a `"ko"` response is penalized with **`-10.0` points**.
- **Contextual Forking and Broadcasting**:
  - Spawning a slot (`FORK`) when empty (`connect_nbr == 0`) and team slots are needed awards **`+2.0` points**; otherwise, it is penalized with **`-2.0` points**.
  - Sending coordinates (`BROADCAST`) when ready for elevation or asking for food when starving is rewarded; unnecessary/spam broadcasting is penalized with **`-0.1` points**.
- **Base Actions & Survival**:
  - Small step reward (`-0.01` per tick) to encourage fast completion.
  - Forward walking (`+0.1`), turning (`+0.02`), and looking (`+0.1`).
  - Dying terminates the episode with a severe penalty of **`-100.0` points**.

### Run AI Training
To launch the autonomous training loop using the high-speed library implementation, you can use the provided bash script. This script automatically builds the Rust engine in release mode and starts the python process with the correct parameters.

```bash
chmod +x ai/training/run_training.sh
./ai/training/run_training.sh
```

You can also customize the training session using command-line arguments:
```bash
./ai/training/run_training.sh -t 100000 -x 20 -y 20 -f 1000 -n TeamAI -m custom_model_name
```
*(Use `./ai/training/run_training.sh --help` to see all available options).*

> **Note on Timesteps:** The Proximal Policy Optimization (PPO) algorithm processes data in batches called "rollouts". By default, Stable-Baselines3 uses a rollout buffer size of **2048 timesteps**. This means even if you request a training session of `-t 10`, the AI will *always* complete at least one full buffer of 2048 steps before performing its first mathematical update and exiting. For a proper training cycle, it is recommended to run at least `-t 2500`.

### Run AI Model

To see your trained model play, you can run the server and connect your AI model client manually.

1. **Start the server**:
   ```bash
   ./zappy_server -p 8080 -x 10 -y 10 -n TeamAI -c 2 -f 10
   ```
2. **Open the GUI**:
   ```bash
   ./zappy_gui -p 8080 -h localhost
   ```
3. **Connect the AI client**:
   From the `ai/` folder, launch the client with the `--ai` flag pointing to your model:
   ```bash
   PYTHONPATH=. python3 src/main.py -p 8080 -n TeamAI -ip 127.0.0.1 --ai --model zappy_ai_model
   ```
   *(To launch multiple AI clients at once, customize variables in [run_bots.sh](file:///Users/anapallares/epi/tek2/zappy/ai/run_bots.sh) and execute it).*

### Run tests
From inside build/
```bash
ctest --output-on-failure
```
