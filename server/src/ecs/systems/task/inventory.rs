use crate::ecs::{
    components::inventory::Inventory,
    storage::{Entity, World},
};
use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};

/// Runs the AI `Inventory` task for `entity`.
///
/// Reads the entity's [`Inventory`] and returns `ok` with a bracket-formatted resource count
/// string. If the component is missing, all resource counts are reported as zero. Does not mutate the world.
pub fn execute_inventory(world: &World, entity: Entity) -> (Response, Option<ServerEvent>) {
    let inventory = world
        .get_component::<Inventory>(entity)
        .cloned()
        .unwrap_or_default();
    (
        Response::new(
            ResponseCode::Status(StatusCode::Ok),
            Some(inventory.format_inventory_response()),
        ),
        None,
    )
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::components::position::Position;
    use crate::ecs::components::resource::Resource;
    use crate::utils::orientation::RelativeOrientation;

    #[test]
    fn execute_inventory_returns_formatted_data_for_inhabitant() {
        let mut world = World::default();
        world.map_size.width = 10;
        world.map_size.height = 10;
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, 2, 3, RelativeOrientation::Forward, &mut world);

        {
            let inv = world.get_component_mut::<Inventory>(entity).unwrap();
            inv.add_item(Resource::Food, 10);
            inv.add_item(Resource::Linemate, 5);
            inv.add_item(Resource::Sibur, 1);
            inv.add_item(Resource::Mendiane, 2);
            inv.add_item(Resource::Phiras, 3);
            inv.add_item(Resource::Thystame, 4);
        }

        let (response, event) = execute_inventory(&world, entity);
        assert_eq!(
            response.data.as_deref(),
            Some("[food 20, linemate 5, deraumere 0, sibur 1, mendiane 2, phiras 3, thystame 4]")
        );
        assert!(event.is_none());
    }

    #[test]
    fn execute_inventory_does_not_change_position_or_orientation() {
        let mut world = World::default();
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, 2, 3, RelativeOrientation::Forward, &mut world);
        let before = (
            world.get_component::<Position>(entity).unwrap().x,
            world.get_component::<Position>(entity).unwrap().y,
            *world.get_component::<RelativeOrientation>(entity).unwrap(),
        );

        execute_inventory(&world, entity);

        let after = (
            world.get_component::<Position>(entity).unwrap().x,
            world.get_component::<Position>(entity).unwrap().y,
            *world.get_component::<RelativeOrientation>(entity).unwrap(),
        );
        assert_eq!(before, after);
    }

    #[test]
    fn execute_inventory_returns_formatted_items() {
        let mut world = World::default();
        let entity = world.spawn();

        let mut inv = Inventory::new();
        inv.add_item(Resource::Food, 10);
        inv.add_item(Resource::Linemate, 5);
        inv.add_item(Resource::Sibur, 1);
        inv.add_item(Resource::Mendiane, 2);
        inv.add_item(Resource::Phiras, 3);
        inv.add_item(Resource::Thystame, 4);
        world.add_component(entity, inv);

        let data = execute_inventory(&world, entity).0.data;
        assert_eq!(
            data.as_deref(),
            Some("[food 10, linemate 5, deraumere 0, sibur 1, mendiane 2, phiras 3, thystame 4]")
        );
    }

    #[test]
    fn execute_inventory_without_component_returns_zeros() {
        let mut world = World::default();
        let entity = world.spawn();

        let (response, _) = execute_inventory(&world, entity);
        assert_eq!(
            response.data.as_deref(),
            Some("[food 0, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0]")
        );
    }
}
