use crate::ecs::{
    components::{inventory::Inventory, position::Position, resource::Resource},
    storage::{Entity, World},
};

/// Marker component to identify a Tile entity.
#[derive(Debug, Default)]
pub struct Tile;

impl Tile {
    /// Transfers a resource from a tile inventory to a destination entity inventory and updates the world's resource count accordingly.
    pub fn take_resource_from_tile(
        tile: Entity,
        world: &mut World,
        resource: Resource,
        destination: Entity,
    ) {
        let tile_inventory = world.get_component_mut::<Inventory>(tile).unwrap();
        tile_inventory.remove_item(resource, 1);
        let destination_inventory = world.get_component_mut::<Inventory>(destination).unwrap();
        destination_inventory.add_item(resource, 1);
        world.resources_amount.remove_item(resource, 1);
    }

    /// Transfers a resource from a source entity inventory to a tile inventory and updates the world's resource count accordingly.
    pub fn drop_resource_on_tile(
        tile: Entity,
        world: &mut World,
        resource: Resource,
        source: Entity,
    ) {
        let tile_inventory = world.get_component_mut::<Inventory>(tile).unwrap();
        tile_inventory.add_item(resource, 1);
        let source_inventory = world.get_component_mut::<Inventory>(source).unwrap();
        source_inventory.remove_item(resource, 1);
        world.resources_amount.add_item(resource, 1);
    }

    pub fn add_resource_to_tile(tile: Entity, world: &mut World, resource: Resource) {
        let tile_inventory = world.get_component_mut::<Inventory>(tile).unwrap();
        tile_inventory.add_item(resource, 1);
        world.resources_amount.add_item(resource, 1);
    }

    pub fn remove_resource_from_tile(tile: Entity, world: &mut World, resource: Resource) {
        let tile_inventory = world.get_component_mut::<Inventory>(tile).unwrap();
        tile_inventory.remove_item(resource, 1);
        world.resources_amount.remove_item(resource, 1);
    }

    /// Finds a tile entity at the given position in the world and returns it if found
    pub fn find_tile_by_pos(position: &Position, world: &mut World) -> Option<Entity> {
        let tile_storage = world.get_storage::<Tile>()?;
        for (entity, _) in tile_storage.iter() {
            if let Some(tile_position) = world.get_component::<Position>(*entity)
                && tile_position == position
            {
                return Some(*entity);
            }
        }
        None
    }

    pub fn trigger_wormhole_if_any(world: &mut World, entity: Entity) {
        use crate::ecs::components::terrain_type::TerrainType;
        use rand::RngExt;

        let pos = match world.get_component::<Position>(entity) {
            Some(p) => p.clone(),
            None => return,
        };

        let tile_ent = match Self::find_tile_by_pos(&pos, world) {
            Some(e) => e,
            None => return,
        };

        let terrain = match world.get_component::<TerrainType>(tile_ent) {
            Some(t) => *t,
            None => return,
        };

        if terrain == TerrainType::Wormhole {
            let mut other_wormholes = Vec::new();
            if let Some(tiles) = world.get_storage::<Tile>() {
                let mut temp_entities = Vec::new();
                for (t_ent, _) in tiles.iter() {
                    if *t_ent != tile_ent {
                        temp_entities.push(*t_ent);
                    }
                }
                for t_ent in temp_entities {
                    if let Some(t) = world.get_component::<TerrainType>(t_ent)
                        && *t == TerrainType::Wormhole
                        && let Some(p) = world.get_component::<Position>(t_ent)
                    {
                        other_wormholes.push(p.clone());
                    }
                }
            }
            if !other_wormholes.is_empty() {
                let mut rng = rand::rng();
                let idx = rng.random_range(0..other_wormholes.len());
                let new_pos = other_wormholes[idx].clone();
                if let Some(mut_pos) = world.get_component_mut::<Position>(entity) {
                    mut_pos.x = new_pos.x;
                    mut_pos.y = new_pos.y;
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::ecs::{builders::tile::build_tile, components::terrain_type::TerrainType};

    use super::*;

    #[test]
    fn find_existing_tile() {
        let mut world = World::default();
        let tile_entity = build_tile(Position { x: 0, y: 0 }, &mut world, TerrainType::Grass);

        let found_entity = Tile::find_tile_by_pos(&Position { x: 0, y: 0 }, &mut world);
        assert_eq!(found_entity, Some(tile_entity));
    }

    #[test]
    fn find_nonexistent_tile() {
        let mut world = World::default();
        let found_entity = Tile::find_tile_by_pos(&Position { x: 0, y: 0 }, &mut world);
        assert_eq!(found_entity, None);
    }
}
