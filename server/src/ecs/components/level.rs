/// Pure data component for storing player level
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Level {
    pub value: u8,
}

impl Level {
    pub fn new() -> Self {
        Level { value: 1 }
    }
}

impl Default for Level {
    fn default() -> Self {
        Self::new()
    }
}
