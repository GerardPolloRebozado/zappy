use crate::{
    ecs::{
        components::inventory::Inventory,
        storage::{Entity, World},
    },
    game::Resource,
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
}
