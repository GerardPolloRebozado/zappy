use rand::{RngExt, rng, seq::SliceRandom};

use std::collections::HashSet;

use crate::{
    ecs::{
        components::{resource::Resource, terrain_type::TerrainType, tile::Tile},
        map_size::get_tile_content_by_entity,
        storage::{Entity, World},
        systems::task::broadcast_event,
    },
    protocol::ServerEvent,
    utils::constants::UNITS_BETWEEN_RESOURCE_SPAWN,
};

/// Populates a tile with resources based on its terrain type. returns true if at least one resource was spawned.
pub fn populate_tile_resources(world: &mut World, tile_ent: &Entity, terrain: TerrainType) -> bool {
    let mut rng = rng();
    let base_density = 0.12;
    let mut spawned_any = false;

    let (food_mult, linemate_mult, deraumere_mult, thystame_mult, phiras_mult) = match terrain {
        TerrainType::ObsidianBarrens => (1.0, 1.5, 1.5, 1.0, 1.0),
        TerrainType::LuminousOrchards => (2.0, 1.0, 1.0, 1.0, 1.0),
        TerrainType::CrystalCanyons => (1.0, 1.0, 1.0, 1.8, 1.5),
        _ => (1.0, 1.0, 1.0, 1.0, 1.0),
    };

    let mut try_spawn = |resource: Resource, multiplier: f32| {
        if (world.resources_amount.get_item_count(resource) as u64)
            < resource.get_resource_max_quantity(world.map_size)
            && rng.random_bool((base_density * multiplier).min(1.0) as f64)
        {
            Tile::add_resource_to_tile(*tile_ent, world, resource);
            spawned_any = true;
        }
    };

    try_spawn(Resource::Food, food_mult);
    try_spawn(Resource::Linemate, linemate_mult);
    try_spawn(Resource::Deraumere, deraumere_mult);
    try_spawn(Resource::Sibur, 1.0);
    try_spawn(Resource::Mendiane, 1.0);
    try_spawn(Resource::Phiras, phiras_mult);
    try_spawn(Resource::Thystame, thystame_mult);

    spawned_any
}

/// System that runs every tick to spawn resources on the map till its full
pub fn resource_spawn_system(world: &mut World) {
    let current_time = world.current_time;
    let ms_between_spawns = (UNITS_BETWEEN_RESOURCE_SPAWN as u128 * 1000) / world.freq as u128;

    if current_time - world.last_resource_spawn < ms_between_spawns as u64 {
        return;
    }

    let mut tile_entities: Vec<(Entity, TerrainType)> =
        if let Some(storage) = world.get_storage::<Tile>() {
            storage
                .iter()
                .filter_map(|(entity, _)| {
                    let terrain = *world.get_component::<TerrainType>(*entity)?;
                    Some((*entity, terrain))
                })
                .collect()
        } else {
            Vec::new()
        };

    let mut rng = rng();
    let mut modified_tiles = HashSet::new();

    loop {
        let mut spawned_this_pass = false;
        let mut all_resources_full = true;

        for res in Resource::iter() {
            if (world.resources_amount.get_item_count(res) as u64)
                < res.get_resource_max_quantity(world.map_size)
            {
                all_resources_full = false;
                break;
            }
        }

        if all_resources_full {
            break;
        }

        tile_entities.shuffle(&mut rng);
        for (entity, terrain) in &tile_entities {
            if populate_tile_resources(world, entity, *terrain) {
                spawned_this_pass = true;
                modified_tiles.insert(*entity);
            }
        }

        if !spawned_this_pass {
            break;
        }
    }

    // Broadcast the changes to the GUI
    for entity in modified_tiles {
        let content = get_tile_content_by_entity(world, entity);
        broadcast_event(world, ServerEvent::TileContent { content });
    }

    world.last_resource_spawn = current_time;
}
