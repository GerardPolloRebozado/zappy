# Zappy Server

The server component of the Zappy project, implemented in Rust. It manages the game world, handles client connections, and enforces game rules.

## Development Setup

### Prerequisites
- **Rust Toolchain:** (Stable version recommended)
- **CMake:** Version 3.28 or higher
- **Corrosion:** (Handled automatically by CMake via FetchContent)

### Compilation

The server is integrated into the root CMake build system. To compile it, run:

```bash
# From the project root
cmake -B build -S .
cmake --build build --target zappy_server
```

The executable `zappy_server` will be generated in the root of your `build` directory.

### Running the Server

You can run the server directly from the build directory:

```bash
./build/zappy_server -p [port] -x [width] -y [height] -n [team_names...] -c [clients_per_team] -t [time_unit]
```

Use `--help` to see all available CLI options:
```bash
./build/zappy_server --help
```

## Project Structure

The server is built:

- **`src/main.rs`**: Entry point. Handles CLI argument parsing and initializes the server.
- **`src/server/`**: Core network logic. Manages the TCP listener, polling, and client communication.
- **`src/game/`**: Implementation of the Zappy game rules, world state, and command execution.
- **`src/ecs/`**: A custom Entity Component System (ECS) used to manage players, eggs, and resources on the map.
- **`src/protocol/`**: Handles the Zappy communication protocol between the server, AI clients, and the GUI.
- **`src/utils/`**: Shared helper functions, logging configuration, and math utilities.
