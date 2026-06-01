//! Direction helpers for the Zappy AI protocol.
//!
//! AI clients receive events such as `message k, text` and `eject: k`, where
//! `k` is a direction relative to the receiving player's orientation.

use crate::game::Player;

/// Rotates a world-map delta into the listener's relative frame.
///
/// Orientation `1`–`4` are north, east, south, and west. Invalid orientations
/// return `(0, 0)`.
fn world_delta_to_relative_offset(orientation: u8, world_x: i32, world_y: i32) -> (i32, i32) {
    match orientation {
        1 => (world_x, world_y),
        2 => (world_y, -world_x),
        3 => (-world_x, -world_y),
        4 => (-world_y, world_x),
        _ => (0, 0),
    }
}

/// Classifies a listener-relative delta into protocol direction `k` (`1`–`8`).
///
/// Expects `(relative_x, relative_y)` already rotated into the listener's frame:
/// forward is negative `y`, back is positive `y`, left is negative `x`, and right
/// is positive `x`. Values `1`–`8` are numbered counter-clockwise from forward.
///
/// Compares `|relative_y|` and `|relative_x|` to pick the closest axis (forward/back
/// or left/right). When both components are equal, the target lies on a 45°
/// diagonal and the signs of `relative_x` and `relative_y` select `k` 2, 4, 6, or 8.
fn classify_relative_direction(relative_x: i32, relative_y: i32) -> u32 {
    let ax = relative_x.abs();
    let ay = relative_y.abs();

    if ay > ax {
        if relative_y < 0 {
            1
        } else {
            5
        }
    } else if ax > ay {
        if relative_x < 0 {
            3
        } else {
            7
        }
    } else {
        match (relative_x.cmp(&0), relative_y.cmp(&0)) {
            (std::cmp::Ordering::Less, std::cmp::Ordering::Less) => 2,
            (std::cmp::Ordering::Less, std::cmp::Ordering::Greater) => 4,
            (std::cmp::Ordering::Greater, std::cmp::Ordering::Greater) => 6,
            (std::cmp::Ordering::Greater, std::cmp::Ordering::Less) => 8,
            _ => 1,
        }
    }
}

/// Returns the shortest signed distance from `from` to `to` on a wrapping axis.
///
/// On a toroidal map of length `size`, the result is the smallest step count
/// in either direction (e.g. on a map of width 10, distance from 9 to 0 is `-1`).
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

/// Computes the relative direction `k` from a player to a map tile.
///
/// The result matches the Zappy protocol: values `1`–`8` for the eight
/// surrounding tiles, and `0` when the target tile is the player's own tile.
/// Direction is expressed relative to the player's current orientation.
///
/// # Examples
///
/// ```
/// use zappy_server::game::Player; use zappy_server::utils::direction::calc_k;
///
/// let player = Player::new(1, 5, 5, 1);
/// assert_eq!(calc_k(5, 4, &player, 10, 10), 1);
/// ```
pub fn calc_k(
    from_x: u32,
    from_y: u32,
    for_player: &Player,
    map_width: u32,
    map_height: u32,
) -> u32 {
    let dx = shortest_delta(for_player.x, from_x, map_width as i32);
    let dy = shortest_delta(for_player.y, from_y, map_height as i32);

    if dx == 0 && dy == 0 {
        return 0;
    }

    let (relative_x, relative_y) = world_delta_to_relative_offset(for_player.orientation, dx, dy);
    if relative_x == 0 && relative_y == 0 {
        return 1;
    }

    classify_relative_direction(relative_x, relative_y)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::game::Player;

    #[test]
    fn same_tile_returns_zero() {
        let player = Player::new(1, 5, 5, 1);
        assert_eq!(calc_k(5, 5, &player, 10, 10), 0);
    }

    #[test]
    fn forward_adjacent_when_facing_north() {
        let player = Player::new(1, 5, 5, 1);
        assert_eq!(calc_k(5, 4, &player, 10, 10), 1);
    }

    #[test]
    fn left_when_facing_east() {
        let player = Player::new(1, 5, 5, 2);
        assert_eq!(calc_k(5, 4, &player, 10, 10), 3);
    }

    #[test]
    fn distant_target_on_cardinal() {
        let player = Player::new(1, 5, 5, 1);
        assert_eq!(calc_k(5, 0, &player, 10, 10), 1);
        assert_eq!(calc_k(9, 5, &player, 10, 10), 7);
    }

    #[test]
    fn distant_target_on_diagonal() {
        let player = Player::new(1, 5, 5, 1);
        assert_eq!(calc_k(9, 1, &player, 10, 10), 8);
        assert_eq!(calc_k(1, 1, &player, 10, 10), 2);
    }

    #[test]
    fn toroidal_shortest_path() {
        let player = Player::new(1, 9, 5, 1);
        assert_eq!(calc_k(0, 5, &player, 10, 10), 7);
        assert_eq!(calc_k(9, 0, &player, 10, 10), 1);
    }

    #[test]
    fn all_eight_directions_facing_north() {
        let player = Player::new(1, 5, 5, 1);
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
        let player = Player::new(1, 5, 5, 9);
        assert_eq!(calc_k(5, 4, &player, 10, 10), 1);
    }
}
