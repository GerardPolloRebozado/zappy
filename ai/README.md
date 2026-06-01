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

### Run tests
From inside build/
```bash
ctest --output-on-failure
```
