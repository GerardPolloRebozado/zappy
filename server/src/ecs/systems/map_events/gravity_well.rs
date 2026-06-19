//! Gravity pull effect for the [`MapEvent::GravityWell`](crate::ecs::map_events::MapEvent::GravityWell) anomaly.
//!
//! Called by the map-event system every 5 ticks while the event is active.
//! Each pull moves eligible inhabitants one tile closer to the vortex center.

use crate::ecs::{
    components::{inhabitant_tag::InhabitantTag, position::Position},
    map_events::{GRAVITY_WELL_PULL_INTERVAL, MapEvent},
    storage::{Entity, World},
};

/// Returns every [`InhabitantTag`] entity with its current tile coordinates.
fn get_inhabitants(world: &World) -> Option<Vec<(Entity, u32, u32)>> {
    let tag_storage = world.get_storage::<InhabitantTag>()?;
    Some(
        tag_storage
            .iter()
            .filter_map(|(entity, _)| {
                let pos = world.get_component::<Position>(*entity)?;
                Some((*entity, pos.x, pos.y))
            })
            .collect(),
    )
}

/// Shortest signed distance from `from` to `to` on a single wrapping axis of length `size`.
///
/// On a toroidal map, the result is the smaller of the direct and wrap-around paths
/// (e.g. on a map of width 10, distance from 9 to 0 is `-1`).
fn toroidal_delta(from: u32, to: u32, size: u32) -> i64 {
    let raw = to as i64 - from as i64;
    if raw.unsigned_abs() as u32 <= size / 2 {
        raw
    } else if raw > 0 {
        raw - size as i64
    } else {
        raw + size as i64
    }
}

/// Shortest signed `(dx, dy)` from `(x, y)` to `(center_x, center_y)` on a toroidal map.
fn toroidal_delta_to(
    x: u32,
    y: u32,
    center_x: u32,
    center_y: u32,
    width: u32,
    height: u32,
) -> (i64, i64) {
    (
        toroidal_delta(x, center_x, width),
        toroidal_delta(y, center_y, height),
    )
}

/// Moves one tile toward `(dx, dy)` from `(x, y)`, wrapping at map edges.
fn step_toward(x: u32, y: u32, dx: i64, dy: i64, width: u32, height: u32) -> (u32, u32) {
    let new_x = ((x as i64 + dx.signum()).rem_euclid(width as i64)) as u32;
    let new_y = ((y as i64 + dy.signum()).rem_euclid(height as i64)) as u32;
    (new_x, new_y)
}

/// Pulls eligible inhabitants one tile closer to the gravity center (GravityWell effect).
pub fn apply_gravity_pull(world: &mut World, center_x: u32, center_y: u32) {
    let width = world.map_size.width;
    let height = world.map_size.height;
    let radius = width.min(height) / 4;

    let Some(entities_to_move) = get_inhabitants(world) else {
        return;
    };

    for (entity, x, y) in entities_to_move {
        let (dx, dy) = toroidal_delta_to(x, y, center_x, center_y, width, height);

        let toroidal_dist = (dx.unsigned_abs() as u32).max(dy.unsigned_abs() as u32);
        if toroidal_dist == 0 || toroidal_dist > radius {
            continue;
        }

        let (new_x, new_y) = step_toward(x, y, dx, dy, width, height);

        if let Some(pos) = world.get_component_mut::<Position>(entity) {
            pos.x = new_x;
            pos.y = new_y;
        }
    }
}

/// Advances the gravity well by one tick: decrements timers, pulls inhabitants when due,
/// and returns `true` when the event duration has elapsed.
pub fn tick(world: &mut World) -> bool {
    let MapEvent::GravityWell {
        ref mut remaining_ticks,
        ref mut ticks_until_next_pull,
        center_x,
        center_y,
    } = world.map_event
    else {
        return false;
    };

    *remaining_ticks = remaining_ticks.saturating_sub(1);
    *ticks_until_next_pull = ticks_until_next_pull.saturating_sub(1);

    if *ticks_until_next_pull == 0 {
        apply_gravity_pull(world, center_x, center_y);
        if let MapEvent::GravityWell {
            ref mut ticks_until_next_pull,
            ..
        } = world.map_event
        {
            *ticks_until_next_pull = GRAVITY_WELL_PULL_INTERVAL;
        }
    }

    matches!(
        world.map_event,
        MapEvent::GravityWell {
            remaining_ticks: 0,
            ..
        }
    )
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::{
        builders::tile::build_tile,
        components::{position::Position, terrain_type::TerrainType},
        storage::World,
    };

    fn world_with_tiles(width: u32, height: u32) -> World {
        let mut world = World::new(crate::ecs::map_size::MapSize { width, height }, 100);
        for y in 0..height {
            for x in 0..width {
                build_tile(Position { x, y }, &mut world, TerrainType::Grass);
            }
        }
        world
    }

    #[test]
    fn pulls_inhabitant_closer() {
        let mut world = world_with_tiles(10, 10);
        let entity = world.spawn();
        world.add_component(entity, InhabitantTag);
        world.add_component(entity, Position { x: 3, y: 3 });

        apply_gravity_pull(&mut world, 5, 5);

        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!(pos.x, 4, "should move one step toward center_x=5");
        assert_eq!(pos.y, 4, "should move one step toward center_y=5");
    }

    #[test]
    fn toroidal_pull() {
        let mut world = world_with_tiles(10, 10);
        let entity = world.spawn();
        world.add_component(entity, InhabitantTag);
        world.add_component(entity, Position { x: 1, y: 1 });

        apply_gravity_pull(&mut world, 9, 9);

        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!(pos.x, 0, "should wrap around toward center_x=9 via x=0");
        assert_eq!(pos.y, 0, "should wrap around toward center_y=9 via y=0");
    }
}
