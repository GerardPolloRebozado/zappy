# MyTeams

A collaborative communication platform consisting of a server and a command-line interface (CLI) client, built with Rust and integrated with a C-based logging library.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Installation & Build](#installation--build)
- [Usage](#usage)
  - [Starting the Server](#starting-the-server)
  - [Starting the Client](#starting-the-client)
- [Available Commands](#available-commands)
- [Project Structure](#project-structure)
- [Technical Specifications](#technical-specifications)
- [Development & Testing](#development--testing)
  - [Running Unit Tests](#running-unit-tests)
  - [Code Coverage](#code-coverage)

## Prerequisites
- **Rust**: Latest stable version (Edition 2024).
- **C Library**: `libmyteams.so` (provided in `libs/myteams/`).
- **Make**: For simplified build commands.

## Installation & Build
The project uses a `Makefile` to handle the build process, including linking the external shared library.

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
The server listens for incoming connections and manages the state of teams, channels, and threads.
```bash
./myteams_server
```

### Starting the Client
The CLI client allows users to interact with the server.
```bash
./myteams_cli [ip] [port]
```

## Available Commands
Once connected via the CLI, the following commands are available:

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `/help` | None | Display this help message. |
| `/login` | `["user_name"]` | Login to the server. |
| `/logout` | None | Logout from the server. |
| `/users` | None | List all users. |
| `/user` | `["user_uuid"]` | Get information about a user. |
| `/send` | `["user_uuid"] ["message_body"]` | Send a private message to a user. |
| `/messages`| `["user_uuid"]` | List all messages with a user. |
| `/subscribe`| `["team_uuid"]` | Subscribe to a team. |
| `/subscribed`| `?["team_uuid"]` | List subscribed teams or users subscribed to a team. |
| `/unsubscribe`| `["team_uuid"]` | Unsubscribe from a team. |
| `/use` | `?["team_uuid"] ?["channel_uuid"] ?["thread_uuid"]` | Set the current context. |
| `/create` | Context dependent | Create a team, channel, thread, or reply. |
| `/list` | Context dependent | List teams, channels, threads, or replies. |
| `/info` | Context dependent | Get info about the current context. |

## Project Structure
- `src/server/`: Server implementation and command handling.
- `src/cli/`: Client implementation and command-line interface.
- `src/common/`: Shared data structures, protocol definitions, and utilities.
- `src/ffi.rs`: Foreign Function Interface for the `myteams` logging library.
- `libs/myteams/`: Contains the pre-compiled shared library and headers.

## Technical Specifications
The following field lengths are strictly enforced:
- **Max Name Length**: 32 characters
- **Max Description Length**: 255 characters
- **Max Body Length**: 512 characters

## Development & Testing

### Running Unit Tests
Unit tests are located within the modules they test (using `#[cfg(test)]`). Because the project links with `libmyteams.so`, you must provide the library path when running tests.

```bash
RUSTFLAGS="-L $(pwd)/libs/myteams -l myteams" cargo test --lib common
```

### Code Coverage
For code coverage, we recommend using **`cargo-llvm-cov`**.

#### 1. Installation
```bash
cargo install cargo-llvm-cov
rustup component add llvm-tools-preview
```

#### 2. View Coverage Summary
```bash
RUSTFLAGS="-L $(pwd)/libs/myteams -l myteams" cargo llvm-cov
```

#### 3. Generate HTML Report
```bash
RUSTFLAGS="-L $(pwd)/libs/myteams -l myteams" cargo llvm-cov --html
```
The report will be available at `target/llvm-cov/html/index.html`.
