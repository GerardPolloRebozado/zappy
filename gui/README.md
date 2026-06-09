# GUI

This is the GUI project for the Multix - Zappy repository.

## Logging

The GUI uses compile-time filtered logging via macros defined in `include/Logging/Logger.hpp`.

### Log macros

| Macro | Output | Description |
|-------|--------|-------------|
| `log_error(msg)` | stderr, `[ERROR]` | Errors and failures |
| `log_debug(msg)` | stdout, `[DEBUG]` | Low-level debug detail (network, input, etc.) |
| `log_info(msg)` | stdout, `[INFO]` | Protocol events and general information |

Include the header and call the macros directly:

```cpp
#include "Logging/Logger.hpp"

log_info("Protocol: connected");
log_debug("Sending command: " + cmd);
log_error("Connection failed");
```

### Log levels at compile time

Verbosity is set with the CMake variable `ZAPPY_LOG_LEVEL` (defined in `gui/CMakeLists.txt`). Higher values include more macros:

| Value | Level | Macros enabled |
|-------|-------|----------------|
| `1` | ERROR | `log_error` only |
| `2` | DEBUG | `log_error`, `log_debug` |
| `3` | INFO | all three (default) |

Macros below the configured level are removed at compile time and produce no code.

### Building with a specific log level

From the repository root:

```bash
cmake -S . -B build -DZAPPY_LOG_LEVEL=2
cmake --build build --target zappy_gui
```

To build only errors:

```bash
cmake -S . -B build -DZAPPY_LOG_LEVEL=1
cmake --build build --target zappy_gui
```

Log level is not a runtime flag on `./zappy_gui`; change it at configure time and rebuild.

### Error handling

GUI exceptions inherit from `errors/IError.hpp` and `errors/AError.hpp`. Domain-specific types live in:

- `CoreErrors.hpp` — CLI / configuration (`ErrorConfig`, `parsePort`)
- `Network/NetworkErrors.hpp` — TCP and I/O (`ErrorNetwork`)
- `Graphics/GraphicsErrors.hpp` — assets (`ErrorAsset`)
- `Commands/CommandsErrors.hpp` — protocol parsing (`ErrorProtocol`)
- `Utils/UtilsErrors.hpp` — utility configuration (`ErrorConfig`)

Catch `zappy::IError` in `main` to handle all GUI errors uniformly.
