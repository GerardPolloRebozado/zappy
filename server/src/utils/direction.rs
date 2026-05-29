//! Direction helpers for the Zappy AI protocol.
//!
//! AI clients receive events such as `message k, text` and `eject: k`, where
//! `k` is a direction relative to the receiving player's orientation.

use crate::game::Player;

const RELATIVE_DIRECTION_OFFSETS: [(i32, i32, u32); 8] = [
    (0, -1, 1),
    (-1, -1, 2),
    (-1, 0, 3),
    (-1, 1, 4),
    (0, 1, 5),
    (1, 1, 6),
    (1, 0, 7),
    (1, -1, 8),
];

/// Rotates a relative `(x, y)` offset into world-map deltas for `orientation`.
///
/// Orientation `1`–`4` are north, east, south, and west. Invalid orientations
/// return `(0, 0)`.
fn relative_offset_to_world_delta(orientation: u8, relative_x: i32, relative_y: i32) -> (i32, i32) {
    match orientation {
        1 => (relative_x, relative_y),
        2 => (-relative_y, relative_x),
        3 => (-relative_x, -relative_y),
        4 => (relative_y, -relative_x),
        _ => (0, 0),
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
///
/// # Note
///
/// Diagonal targets more than one tile away are not fully handled yet;
/// the function currently falls back to `1` in those cases.
// TODO: Handle edge cases when source is more than one tile away on diagonals
// We could have a simple way of doing it without the diagonals,
// meaning that we can do straight paths (its not perfect but enough)
// or implement the fully working version with diagonals for messages because the math part is pretty hard
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
    for (relative_x, relative_y, direction_k) in RELATIVE_DIRECTION_OFFSETS {
        let (world_delta_x, world_delta_y) =
            relative_offset_to_world_delta(for_player.orientation, relative_x, relative_y);
        if world_delta_x == dx && world_delta_y == dy {
            return direction_k;
        }
    }
    1
}
