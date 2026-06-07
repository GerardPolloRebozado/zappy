use rand::{RngExt, rng};

use crate::{
    ecs::{
        components::{inventory::Inventory, terrain_type::TerrainType, tile::Tile},
        storage::{Entity, World},
    },
    game::{Date, Resource},
    utils::constants::UNITS_BETWEEN_RESOURCE_SPAWN,
};

/// Populates a tile with resources based on its terrain type.
pub fn populate_tile_resources(world: &mut World, tile_ent: &Entity, terrain: TerrainType) {
    let mut rng = rng();
    let base_density = 0.12; // Base probability of a resource spawning on a tile

    let (food_mult, linemate_mult, deraumere_mult, thystame_mult, phiras_mult) = match terrain {
        TerrainType::ObsidianBarrens => (0.2, 1.5, 1.5, 1.0, 1.0),
        TerrainType::LuminousOrchards => (2.0, 1.0, 1.0, 1.0, 1.0),
        TerrainType::CrystalCanyons => (1.0, 1.0, 1.0, 1.8, 1.5),
        _ => (1.0, 1.0, 1.0, 1.0, 1.0),
    };

    let mut resources = Vec::new();

    // Helper to roll for resource
    let mut try_spawn = |resource: Resource, multiplier: f32| {
        if resource.get_resource_max_quantity(world.map_size)
            <= (world.resources_amount.get_item_count(resource)) as u64
        {
            return;
        }
        if rng.random_bool((base_density * multiplier).min(1.0) as f64) {
            resources.push(resource);
            world.resources_amount.add_item(resource, 1);
        }
    };

    try_spawn(Resource::Food, food_mult);
    try_spawn(Resource::Linemate, linemate_mult);
    try_spawn(Resource::Deraumere, deraumere_mult);
    try_spawn(Resource::Sibur, 1.0); // same possibility everywhere
    try_spawn(Resource::Mendiane, 1.0);
    try_spawn(Resource::Phiras, phiras_mult);
    try_spawn(Resource::Thystame, thystame_mult);

    if let Some(inventory) = world.get_component_mut::<Inventory>(*tile_ent) {
        for res in resources {
            *inventory.items.entry(res).or_insert(0) += 1;
        }
    }
}

pub fn resource_spawn_system(world: &mut World) {
    let current_time = Date::now().to_timestamp();
    let seconds_between_spawns = UNITS_BETWEEN_RESOURCE_SPAWN / world.freq;
    if current_time - world.last_resource_spawn < seconds_between_spawns {
        return;
    }

    let entities: Vec<Entity> = if let Some(storage) = world.get_storage::<Tile>() {
        storage.iter().map(|(entity, _)| *entity).collect()
    } else {
        Vec::new()
    };

    for entity in entities {
        populate_tile_resources(world, &entity, TerrainType::CrystalCanyons);
    }
    world.last_resource_spawn = current_time;
}
