use crate::ecs::{
    components::inventory::Inventory,
    storage::{Entity, World},
    systems::inventory_system::format_inventory_response,
};

/// Runs the AI `Inventory` task for `entity`.
///
/// Reads the entity's [`Inventory`] and returns the bracket-formatted response string
/// via [`format_inventory_response`]. If the component is missing, all resource counts
/// are reported as zero. Does not mutate the world.
pub fn execute_inventory(world: &World, entity: Entity) -> String {
    let inventory = world
        .get_component::<Inventory>(entity)
        .cloned()
        .unwrap_or_default();
    format_inventory_response(&inventory)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::systems::inventory_system::add_item;
    use crate::game::Resource;

    #[test]
    fn execute_inventory_returns_formatted_items() {
        let mut world = World::default();
        let entity = world.spawn();

        let mut inv = Inventory::new();
        add_item(&mut inv, Resource::Food, 10);
        add_item(&mut inv, Resource::Linemate, 5);
        add_item(&mut inv, Resource::Sibur, 1);
        add_item(&mut inv, Resource::Mendiane, 2);
        add_item(&mut inv, Resource::Phiras, 3);
        add_item(&mut inv, Resource::Thystame, 4);
        world.add_component(entity, inv);

        let data = execute_inventory(&world, entity);
        assert_eq!(
            data,
            "[food 10, linemate 5, deraumere 0, sibur 1, mendiane 2, phiras 3, thystame 4]"
        );
    }

    #[test]
    fn execute_inventory_without_component_returns_zeros() {
        let mut world = World::default();
        let entity = world.spawn();

        let data = execute_inventory(&world, entity);
        assert_eq!(
            data,
            "[food 0, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0]"
        );
    }
}
