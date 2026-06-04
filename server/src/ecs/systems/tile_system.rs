use crate::ecs::components::inventory::Inventory;
use crate::ecs::components::position::Position;
use crate::ecs::components::terrain_type::TerrainType;
use crate::ecs::components::tile::Tile;
use crate::ecs::map_size::MapSize;
use crate::ecs::storage::{Entity, World};
use crate::game::Resource;
use noise::{NoiseFn, Perlin};
use rand::{RngExt, rng};

/// Spawns a single tile entity with the required components.
pub fn spawn_tile(world: &mut World, x: u32, y: u32, terrain: TerrainType) -> Entity {
    let entity = world.spawn();
    world.add_component(entity, Tile);
    world.add_component(entity, Position { x, y });
    world.add_component(entity, terrain);
    world.add_component(entity, Inventory::default());
    entity
}

/// Populates a tile with resources based on its terrain type.
pub fn populate_tile_resources(world: &mut World, tile_ent: Entity, terrain: TerrainType) {
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
        if rng.random_bool((base_density * multiplier).min(1.0) as f64) {
            resources.push(resource);
        }
    };

    try_spawn(Resource::Food, food_mult);
    try_spawn(Resource::Linemate, linemate_mult);
    try_spawn(Resource::Deraumere, deraumere_mult);
    try_spawn(Resource::Sibur, 1.0); // same possibility everywhere
    try_spawn(Resource::Mendiane, 1.0);
    try_spawn(Resource::Phiras, phiras_mult);
    try_spawn(Resource::Thystame, thystame_mult);

    if let Some(inventory) = world.get_component_mut::<Inventory>(tile_ent) {
        for res in resources {
            *inventory.items.entry(res).or_insert(0) += 1;
        }
    }
}

/// Initializes the game map by registering components and spawning a grid of tiles using noise
pub fn setup_map(world: &mut World, width: u32, height: u32) {
    world.mapSize = MapSize { width, height };
    world.register_component::<Tile>();
    world.register_component::<Position>();
    world.register_component::<TerrainType>();
    world.register_component::<Inventory>();

    let mut rng = rng();
    let seed = rng.random();
    //    println!("Map Generation: Seed = {}", seed);
    let perlin_elevation = Perlin::new(seed);
    let perlin_moisture = Perlin::new(seed.wrapping_add(1));

    let scale = 0.08; // Increased scale for more varied biomes

    for y in 0..height {
        for x in 0..width {
            let nx = x as f64 * scale;
            let ny = y as f64 * scale;

            // Get noise values and normalize roughly to [0, 1]
            let elev = (perlin_elevation.get([nx, ny]) + 1.0) / 2.0;
            let moist = (perlin_moisture.get([nx, ny]) + 1.0) / 2.0;

            let terrain = if elev < 0.25 {
                TerrainType::Water
            } else if elev > 0.85 {
                TerrainType::Mountain
            } else if elev > 0.75 {
                TerrainType::ObsidianBarrens
            } else {
                // Mid-elevation biomes based on moisture
                if moist < 0.2 {
                    TerrainType::Sand
                } else if moist < 0.4 {
                    TerrainType::Grass
                } else if moist < 0.6 {
                    TerrainType::Forest
                } else if moist < 0.75 {
                    TerrainType::MagneticTundra
                } else if moist < 0.9 {
                    TerrainType::LuminousOrchards
                } else {
                    TerrainType::CrystalCanyons
                }
            };

            let tile_ent = spawn_tile(world, x, y, terrain);
            populate_tile_resources(world, tile_ent, terrain);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_spawn_tile() {
        let mut world = World::new();

        let tile_ent = spawn_tile(&mut world, 10, 20, TerrainType::Grass);

        assert!(world.is_alive(tile_ent));
        assert!(world.get_component::<Tile>(tile_ent).is_some());

        let pos = world.get_component::<Position>(tile_ent).unwrap();
        assert_eq!(pos.x, 10);
        assert_eq!(pos.y, 20);

        let terrain = world.get_component::<TerrainType>(tile_ent).unwrap();
        match terrain {
            TerrainType::Grass => (),
            _ => panic!("Expected Grass terrain"),
        }

        assert!(world.get_component::<Inventory>(tile_ent).is_some());
    }

    #[test]
    fn test_setup_map() {
        let mut world = World::new();
        setup_map(&mut world, 10, 10);

        let tiles = world.get_storage::<Tile>().unwrap();
        assert_eq!(tiles.iter().count(), 100);

        // Check a specific tile
        let pos_storage = world.get_storage::<Position>().unwrap();
        let mut found = false;
        for (ent, pos) in pos_storage.iter() {
            if pos.x == 5 && pos.y == 5 {
                assert!(world.get_component::<Tile>(*ent).is_some());
                found = true;
                break;
            }
        }
        assert!(found);
    }
}
