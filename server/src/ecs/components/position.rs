#[derive(Debug, Default)]
pub struct Position {
    pub x: u32,
    pub y: u32,
}

impl Position {
    pub fn new() -> Position {
        Position { x: 0, y: 0 }
    }
}
