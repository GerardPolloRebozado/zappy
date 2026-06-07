use rand::{RngExt, rng, seq::SliceRandom};

use crate::{
    ecs::{
        components::{terrain_type::TerrainType, tile::Tile},
        storage::{Entity, World},
    },
    game::{Date, Resource},
    utils::constants::UNITS_BETWEEN_RESOURCE_SPAWN,
};

/// Populates a tile with resources based on its terrain type. returns true if at least one resource was spawned.
pub fn populate_tile_resources(world: &mut World, tile_ent: &Entity, terrain: TerrainType) -> bool {
    let mut rng = rng();
    let base_density = 0.12;
    let mut spawned_any = false;

    let (food_mult, linemate_mult, deraumere_mult, thystame_mult, phiras_mult) = match terrain {
        TerrainType::ObsidianBarrens => (0.2, 1.5, 1.5, 1.0, 1.0),
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
    let current_time = Date::now().to_timestamp();
    let seconds_between_spawns = UNITS_BETWEEN_RESOURCE_SPAWN / world.freq;

    if current_time - world.last_resource_spawn < seconds_between_spawns {
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
            }
        }

        if !spawned_this_pass {
            break;
        }
    }

    world.last_resource_spawn = current_time;
}
