# AI

This is the AI project for the Multis - Zappy repository.

## Project Structure

The source code is organized into a layered architecture:

- **`src/network/`**: The communication layer. Handles TCP sockets and ensures reliable line-by-line data exchange with the server.
- **`src/client/`**: The protocol layer. Defines the `ZappyAiClient` class and maps high-level actions to the specific command strings expected by the server.
- **`src/strategy/`**: The logic layer. This is where the AI's "brain" resides, containing the main loop and decision making algorithms.
- **`src/utils/`**: Utility functions and classes that support the other layers.
- **`src/main.py`**: The entry point. Manages command-line argument parsing and initializes the client connection.

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
- **Action Space (What the AI does):** 19 actions, mapped to Zappy commands via an `IntEnum`. 

### The Rewards System
The Reinforcement Learning agent starts blind and learns optimal behavior by trying to maximize its cumulative score based on the following ruleset:

- **`+1.0` (Success):** Awarded when the server returns `ok`. The AI is encouraged to perform valid, actionable commands (like moving successfully or picking up an item that actually exists).
- **`-0.1` (Failure):** Penalized when the server returns `ko`. The AI learns to avoid useless actions (like trying to pick up food from an empty tile or walking into walls).
- **`-0.5` (Invalid Action):** Penalized if the neural network attempts to use an unmapped/invalid action ID.
- **`-100.0` (Death/Termination):** The ultimate punishment. If the player runs out of life units (1260 ticks without eating) and the server sends a `dead` signal (Broken Pipe), the episode terminates with a massive penalty. This forces the AI to prioritize food collection and survival above all else.

### Run AI Training
To launch the autonomous training loop (which automatically spins up a background Zappy Server instance and manages the lifecycle), you first need to configure the build directory:

```bash
cd build
cmake ..
make train
```

### Run AI Model  

```bash
python -m training.training_env.watch_ai
#on different terminals
./zappy_gui -p 8080 -h localhost
```

### Run tests
From inside build/
```bash
ctest --output-on-failure
```
