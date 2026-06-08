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

### Run tests
From inside build/
```bash
ctest --output-on-failure
```
