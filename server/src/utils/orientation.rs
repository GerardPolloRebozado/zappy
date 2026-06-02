//! Orientation helpers for the Zappy AI protocol.
//!
//! AI clients receive events such as `message k, text` and `eject: k`, where
//! `k` is a orientation relative to the receiving player's orientation.

use std::cmp::Ordering;

use crate::game::Inhabitant;

/// Orientation from a listener to a source tile, relative to the listener's facing.
///
/// Values match the Zappy AI protocol `k` field in `message k, text` and `eject: k`.
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RelativeOrientation {
    SameTile = 0,
    Forward = 1,
    ForwardLeft = 2,
    Left = 3,
    BackLeft = 4,
    Back = 5,
    BackRight = 6,
    Right = 7,
    ForwardRight = 8,
    Invalid = 9,
}

impl RelativeOrientation {
    /// Returns the protocol `k` integer sent to AI clients.
    pub fn as_protocol_k(self) -> u32 {
        self as u32
    }
}

/// Rotates a world-map delta into the listener's relative frame.
///
/// Orientation `1`–`4` are north, east, south, and west. Invalid orientations
/// return `(0, 0)`.
fn world_delta_to_relative_offset(
    orientation: RelativeOrientation,
    world_x: i32,
    world_y: i32,
) -> (i32, i32) {
    match orientation.as_protocol_k() {
        1 => (world_x, world_y),
        2 => (world_y, -world_x),
        3 => (-world_x, -world_y),
        4 => (-world_y, world_x),
        _ => (0, 0),
    }
}

/// Classifies a listener-relative delta into protocol orientation `k` (`1`–`8`).
///
/// Expects `(relative_x, relative_y)` already rotated into the listener's frame:
/// forward is negative `y`, back is positive `y`, left is negative `x`, and right
/// is positive `x`.
///
/// Compares `|relative_y|` and `|relative_x|` to pick the closest axis (forward/back
/// or left/right). When both components are equal, the target lies on a 45°
/// diagonal and the signs of `relative_x` and `relative_y` select the diagonal variants.
fn classify_relative_orientation(relative_x: i32, relative_y: i32) -> RelativeOrientation {
    let ax = relative_x.abs();
    let ay = relative_y.abs();

    if ay > ax {
        if relative_y < 0 {
            RelativeOrientation::Forward
        } else {
            RelativeOrientation::Back
        }
    } else if ax > ay {
        if relative_x < 0 {
            RelativeOrientation::Left
        } else {
            RelativeOrientation::Right
        }
    } else {
        match (relative_x.cmp(&0), relative_y.cmp(&0)) {
            (Ordering::Less, Ordering::Less) => RelativeOrientation::ForwardLeft,
            (Ordering::Less, Ordering::Greater) => RelativeOrientation::BackLeft,
            (Ordering::Greater, Ordering::Greater) => RelativeOrientation::BackRight,
            (Ordering::Greater, Ordering::Less) => RelativeOrientation::ForwardRight,
            _ => RelativeOrientation::Forward,
        }
    }
}

/// Returns the shortest signed distance from `from` to `to` on a wrapping axis.
///
/// On a toroidal map of length `size`, the result is the smallest step count
/// in either orientation (e.g. on a map of width 10, distance from 9 to 0 is `-1`).
fn shortest_delta(from: u32, to: u32, size: i32) -> i32 {
    let diff = to as i32 - from as i32;
    if diff > size / 2 {
        diff - size
    } else if diff < -(size / 2) {
        diff + size
    } else {
        diff
    }
}

/// Computes the relative orientation `k` from an inhabitant to a map tile.
///
/// The result matches the Zappy protocol: values `1`–`8` for the eight
/// surrounding tiles, and `0` when the target tile is the inhabitant's own tile.
/// Orientation is expressed relative to the inhabitant's current orientation.
///
/// # Examples
///
/// ```
/// use zappy_server::game::Inhabitant; use zappy_server::utils::orientation::calc_k;
/// use zappy_server::utils::orientation::RelativeOrientation;
///
/// let player = Inhabitant::new(1, 5, 5, RelativeOrientation::Forward);
/// assert_eq!(calc_k(5, 4, &player, 10, 10), 1);
/// ```
pub fn calc_k(
    from_x: u32,
    from_y: u32,
    for_player: &Inhabitant,
    map_width: u32,
    map_height: u32,
) -> u32 {
    let dx = shortest_delta(for_player.x(), from_x, map_width as i32);
    let dy = shortest_delta(for_player.y(), from_y, map_height as i32);

    if dx == 0 && dy == 0 {
        return RelativeOrientation::SameTile.as_protocol_k();
    }

    let (relative_x, relative_y) = world_delta_to_relative_offset(for_player.orientation(), dx, dy);
    if relative_x == 0 && relative_y == 0 {
        return RelativeOrientation::Forward.as_protocol_k();
    }

    classify_relative_orientation(relative_x, relative_y).as_protocol_k()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::game::Inhabitant;

    #[test]
    fn same_tile_returns_zero() {
        let player = Inhabitant::new(1, 5, 5, RelativeOrientation::Forward);
        assert_eq!(calc_k(5, 5, &player, 10, 10), 0);
    }

    #[test]
    fn forward_adjacent_when_facing_north() {
        let player = Inhabitant::new(1, 5, 5, RelativeOrientation::Forward);
        assert_eq!(calc_k(5, 4, &player, 10, 10), 1);
    }

    #[test]
    fn left_when_facing_east() {
        let player = Inhabitant::new(1, 5, 5, RelativeOrientation::ForwardLeft);
        assert_eq!(calc_k(5, 4, &player, 10, 10), 3);
    }

    #[test]
    fn distant_target_on_cardinal() {
        let player = Inhabitant::new(1, 5, 5, RelativeOrientation::Forward);
        assert_eq!(calc_k(5, 0, &player, 10, 10), 1);
        assert_eq!(calc_k(9, 5, &player, 10, 10), 7);
    }

    #[test]
    fn distant_target_on_diagonal() {
        let player = Inhabitant::new(1, 5, 5, RelativeOrientation::Forward);
        assert_eq!(calc_k(9, 1, &player, 10, 10), 8);
        assert_eq!(calc_k(1, 1, &player, 10, 10), 2);
    }

    #[test]
    fn toroidal_shortest_path() {
        let player = Inhabitant::new(1, 9, 5, RelativeOrientation::Forward);
        assert_eq!(calc_k(0, 5, &player, 10, 10), 7);
        assert_eq!(calc_k(9, 0, &player, 10, 10), 1);
    }

    #[test]
    fn all_eight_orientation_facing_north() {
        let player = Inhabitant::new(1, 5, 5, RelativeOrientation::Forward);
        assert_eq!(calc_k(5, 4, &player, 10, 10), 1);
        assert_eq!(calc_k(4, 4, &player, 10, 10), 2);
        assert_eq!(calc_k(4, 5, &player, 10, 10), 3);
        assert_eq!(calc_k(4, 6, &player, 10, 10), 4);
        assert_eq!(calc_k(5, 6, &player, 10, 10), 5);
        assert_eq!(calc_k(6, 6, &player, 10, 10), 6);
        assert_eq!(calc_k(6, 5, &player, 10, 10), 7);
        assert_eq!(calc_k(6, 4, &player, 10, 10), 8);
    }

    #[test]
    fn invalid_orientation_falls_back_to_one() {
        let player = Inhabitant::new(1, 5, 5, RelativeOrientation::Invalid);
        assert_eq!(calc_k(5, 4, &player, 10, 10), 1);
    }

    #[test]
    fn relative_orientation_matches_protocol_k() {
        assert_eq!(RelativeOrientation::SameTile.as_protocol_k(), 0);
        assert_eq!(RelativeOrientation::Forward.as_protocol_k(), 1);
        assert_eq!(RelativeOrientation::ForwardLeft.as_protocol_k(), 2);
        assert_eq!(RelativeOrientation::Left.as_protocol_k(), 3);
        assert_eq!(RelativeOrientation::BackLeft.as_protocol_k(), 4);
        assert_eq!(RelativeOrientation::Back.as_protocol_k(), 5);
        assert_eq!(RelativeOrientation::BackRight.as_protocol_k(), 6);
        assert_eq!(RelativeOrientation::Right.as_protocol_k(), 7);
        assert_eq!(RelativeOrientation::ForwardRight.as_protocol_k(), 8);
    }
}
