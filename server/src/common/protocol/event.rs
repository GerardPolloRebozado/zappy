#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EventCode {
    Message,
    Eject,
    Dead,

    Pnw,
    Pdi,
    Ppo,
    Pie,
    Seg,
}

// TODO: Refactor into a `ServerEvent` enum that represents logical game occurrences.
// This new enum should support dual-formatting:
// - `.to_ai_string()`: For AI clients (e.g., "eject: K\n")
// - `.to_gui_string()`: For GUI clients (e.g., "pex n\n")
// This will replace the flat `EventCode` mapping.
