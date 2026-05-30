use crate::ecs::components::inventory::Inventory;
use crate::ecs::components::position::Position;
use crate::ecs::components::terrain_type::TerrainType;
use crate::ecs::components::tile::Tile;
use crate::ecs::storage::{Entity, World};

/// Spawns a single tile entity with the required components.
pub fn spawn_tile(world: &mut World, x: u32, y: u32, terrain: TerrainType) -> Entity {
    let entity = world.spawn();
    world.add_component(entity, Tile);
    world.add_component(entity, Position { x, y });
    world.add_component(entity, terrain);
    world.add_component(entity, Inventory::default());
    entity
}

/// Initializes the game map by registering components and spawning a grid of tiles.
pub fn setup_map(world: &mut World, width: u32, height: u32) {
    world.register_component::<Tile>();
    world.register_component::<Position>();
    world.register_component::<TerrainType>();
    world.register_component::<Inventory>();

    for y in 0..height {
        for x in 0..width {
            spawn_tile(world, x, y, TerrainType::Grass);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_spawn_tile() {
        let mut world = World::new();
        world.register_component::<Tile>();
        world.register_component::<Position>();
        world.register_component::<TerrainType>();
        world.register_component::<Inventory>();

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
