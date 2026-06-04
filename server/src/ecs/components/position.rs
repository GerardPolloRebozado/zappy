use crate::utils::orientation::RelativeOrientation;

#[derive(Debug, Default)]
pub struct Position {
    pub x: u32,
    pub y: u32,
}

impl Position {
    pub fn new() -> Position {
        Position { x: 0, y: 0 }
    }

    /// Steps one tile along the inhabitant's facing (`Forward` = north, `ForwardLeft` = east,
    /// `Left` = south, `BackLeft` = west). Other orientations leave the position unchanged.
    /// Coordinates wrap on both axes.
    pub fn move_forward(
        &mut self,
        orientation: RelativeOrientation,
        map_width: u32,
        map_height: u32,
    ) {
        match orientation {
            RelativeOrientation::Forward => {
                self.y = (self.y + map_height - 1) % map_height;
            }
            RelativeOrientation::ForwardLeft => {
                self.x = (self.x + 1) % map_width;
            }
            RelativeOrientation::Left => {
                self.y = (self.y + 1) % map_height;
            }
            RelativeOrientation::BackLeft => {
                self.x = (self.x + map_width - 1) % map_width;
            }
            _ => {}
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn move_forward_north() {
        let mut pos = Position { x: 5, y: 5 };
        pos.move_forward(RelativeOrientation::Forward, 10, 10);
        assert_eq!((pos.x, pos.y), (5, 4));
    }

    #[test]
    fn move_forward_east() {
        let mut pos = Position { x: 5, y: 5 };
        pos.move_forward(RelativeOrientation::ForwardLeft, 10, 10);
        assert_eq!((pos.x, pos.y), (6, 5));
    }

    #[test]
    fn move_forward_south() {
        let mut pos = Position { x: 5, y: 5 };
        pos.move_forward(RelativeOrientation::Left, 10, 10);
        assert_eq!((pos.x, pos.y), (5, 6));
    }

    #[test]
    fn move_forward_west() {
        let mut pos = Position { x: 5, y: 5 };
        pos.move_forward(RelativeOrientation::BackLeft, 10, 10);
        assert_eq!((pos.x, pos.y), (4, 5));
    }

    #[test]
    fn move_forward_wraps_north() {
        let mut pos = Position { x: 3, y: 0 };
        pos.move_forward(RelativeOrientation::Forward, 10, 10);
        assert_eq!((pos.x, pos.y), (3, 9));
    }

    #[test]
    fn move_forward_wraps_east() {
        let mut pos = Position { x: 9, y: 3 };
        pos.move_forward(RelativeOrientation::ForwardLeft, 10, 10);
        assert_eq!((pos.x, pos.y), (0, 3));
    }

    #[test]
    fn move_forward_wraps_south() {
        let mut pos = Position { x: 3, y: 9 };
        pos.move_forward(RelativeOrientation::Left, 10, 10);
        assert_eq!((pos.x, pos.y), (3, 0));
    }

    #[test]
    fn move_forward_wraps_west() {
        let mut pos = Position { x: 0, y: 3 };
        pos.move_forward(RelativeOrientation::BackLeft, 10, 10);
        assert_eq!((pos.x, pos.y), (9, 3));
    }
}
