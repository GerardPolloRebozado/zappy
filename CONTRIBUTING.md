# Contributing

## Branch Naming
- Format: `[issue_number]-[description]`
- Example: `12-add-server-multiplexing`

## Pull Requests
- Mandatory approval from Project Managers: Alejandro Laguna or Gerard Du Pre.
- For domain-specific changes, or if you believe someone else should provide input on that PR, add _also_ the respective expert as a reviewer.
- Link the related issue(s).
- Ensure all binaries (`zappy_server`, `zappy_gui`, `zappy_ai`) work and compile (there will be GitHub actions for it too).
- Do not commit binaries or build artifacts.

## Standards
- **Testing**: Every PR must include unit tests. The specific framework is TBD, but coverage is required.
- **Documentation**: Use Doxygen or an equivalent tool for code documentation where applicable.
- **Formatting**: Enforced via git hooks using `.clang-format`.
