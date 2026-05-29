/// A player on the game map.
///
/// Holds the minimum state needed to format AI and GUI protocol messages,
/// such as broadcast direction (`k`) and player spawn notifications.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Player {
    pub id: u32,
    pub x: u32,
    pub y: u32,
    /// Facing direction on the map (`1` = north, `2` = east, `3` = south, `4` = west).
    pub orientation: u8,
}

impl Player {
    /// Creates a player at the given position and orientation.
    ///
    /// # Examples
    ///
    /// ```
    /// use zappy_server::game::Player;
    ///
    /// let player = Player::new(1, 5, 5, 1);
    /// assert_eq!(player.id, 1);
    /// assert_eq!(player.orientation, 1);
    /// ```
    pub fn new(id: u32, x: u32, y: u32, orientation: u8) -> Self {
        Self {
            id,
            x,
            y,
            orientation,
        }
    }
}
