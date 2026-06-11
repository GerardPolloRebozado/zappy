use crate::ecs::builders::tile::build_tile;
use crate::ecs::components::{egg::Egg, position::Position, terrain_type::TerrainType};
use crate::ecs::map_size::MapSize;
use crate::ecs::storage::World;
use crate::ecs::systems::resource_spawn::resource_spawn_system;
use crate::ecs::systems::task::broadcast_event;
use crate::protocol::ServerEvent;
use log::info;
use noise::{NoiseFn, Perlin};
use rand::{RngExt, rng};

/// Initializes the game map by registering components and spawning a grid of tiles using noise
pub fn setup_map(world: &mut World, width: u32, height: u32) {
    world.map_size = MapSize { width, height };

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

            build_tile(Position { x, y }, world, terrain);
        }
    }
    resource_spawn_system(world);
}

pub fn spawn_egg(size: MapSize, world: &mut World, player_id: u32) {
    let mut rng = rng();
    let x = rng.random_range(0..size.width);
    let y = rng.random_range(0..size.height);
    let entity = world.spawn();
    world.add_component(entity, Egg);
    world.add_component(entity, Position { x, y });
    info!("Spawned egg at x: {} y: {}", x, y);
    broadcast_event(
        world,
        ServerEvent::EggLaid {
            egg_id: entity.id(),
            player_id,
            x,
            y,
        },
    );
}

#[cfg(test)]
mod tests {
    use crate::ecs::components::{inventory::Inventory, tile::Tile};

    use super::*;

    #[test]
    fn test_spawn_tile() {
        let mut world = World::default();

        let tile_ent = build_tile(Position { x: 10, y: 20 }, &mut world, TerrainType::Grass);

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
        let mut world = World::default();
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

    #[test]
    fn test_spawn_egg() {
        let mut world = World::default();
        let size = MapSize {
            width: 10,
            height: 10,
        };

        spawn_egg(size, &mut world, 42);

        let egg_storage = world
            .get_storage::<Egg>()
            .expect("Egg storage should exist");
        assert_eq!(egg_storage.iter().count(), 1);

        let (egg_ent, _) = egg_storage.iter().next().unwrap();
        let pos = world
            .get_component::<Position>(*egg_ent)
            .expect("Egg should have a position");

        assert!(pos.x < 10);
        assert!(pos.y < 10);
    }
}
