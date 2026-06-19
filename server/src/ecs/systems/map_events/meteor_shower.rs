use rand::{RngExt, rng};

use crate::ecs::{
    components::{resource::Resource, tile::Tile},
    map_events::{METEOR_SHOWER_SPAWN_INTERVAL, MapEvent},
    storage::{Entity, World},
};

/// Spawns `(W * H * 0.05)` random gems on random tiles.
pub fn spawn_meteor_gems(world: &mut World) {
    let width = world.map_size.width;
    let height = world.map_size.height;
    let count = ((width as f64 * height as f64) * 0.05).ceil() as u32;

    let tile_entities: Vec<Entity> = if let Some(storage) = world.get_storage::<Tile>() {
        storage.iter().map(|(e, _)| *e).collect()
    } else {
        return;
    };

    if tile_entities.is_empty() {
        return;
    }

    let gem_types: Vec<Resource> = Resource::iter().filter(|r| *r != Resource::Food).collect();

    let mut rng = rng();
    for _ in 0..count {
        let tile_idx = rng.random_range(0..tile_entities.len());
        let gem = gem_types[rng.random_range(0..gem_types.len())];
        Tile::add_resource_to_tile(tile_entities[tile_idx], world, gem);
    }
}

/// Advances the meteor shower by one tick: decrements timers, spawns gems when due,
/// and returns `true` when the event duration has elapsed.
pub fn tick(world: &mut World) -> bool {
    let MapEvent::MeteorShower {
        ref mut remaining_ticks,
        ref mut ticks_until_next_spawn,
    } = world.map_event
    else {
        return false;
    };

    *remaining_ticks = remaining_ticks.saturating_sub(1);
    *ticks_until_next_spawn = ticks_until_next_spawn.saturating_sub(1);

    if *ticks_until_next_spawn == 0 {
        spawn_meteor_gems(world);
        if let MapEvent::MeteorShower {
            ref mut ticks_until_next_spawn,
            ..
        } = world.map_event
        {
            *ticks_until_next_spawn = METEOR_SHOWER_SPAWN_INTERVAL;
        }
    }

    matches!(
        world.map_event,
        MapEvent::MeteorShower {
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
    fn spawn_meteor_gems_adds_gems_to_tiles() {
        let mut world = world_with_tiles(10, 10);

        let before: u32 = Resource::iter()
            .filter(|r| *r != Resource::Food)
            .map(|r| world.resources_amount.get_item_count(r))
            .sum();

        spawn_meteor_gems(&mut world);

        let after: u32 = Resource::iter()
            .filter(|r| *r != Resource::Food)
            .map(|r| world.resources_amount.get_item_count(r))
            .sum();

        assert!(after > before, "gems should have been spawned");
    }
}
