# Zappy Server

A barebones network server built with Rust, intended as a starting point for the Zappy project.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Installation & Build](#installation--build)
- [Usage](#usage)
  - [Starting the Server](#starting-the-server)
  - [Starting the Client](#starting-the-client)
- [Available Commands](#available-commands)
- [Project Structure](#project-structure)

## Prerequisites
- **Rust**: Latest stable version (Edition 2024).
- **Make**: For simplified build commands.

## Installation & Build
The project uses a `Makefile` to handle the build process.

```bash
# Build both server and client
make

# Clean build artifacts
make clean

# Full cleanup (including binaries)
make fclean
```

## Usage

### Starting the Server
The server listens for incoming connections.
```bash
./zappy_server
```

### Starting the Client
The CLI client allows users to interact with the server.
```bash
./zappy_cli [ip] [port]
```

## Available Commands
Once connected via the CLI, the following commands are available:

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `/help` | None | Display this help message. |
| `/login` | `["user_name"]` | Login to the server. |
| `/logout` | None | Logout from the server. |

## Project Structure
- `src/server/`: Server implementation and command handling.
- `src/cli/`: Client implementation and command-line interface.
- `src/common/`: Shared data structures, protocol definitions, and utilities.
