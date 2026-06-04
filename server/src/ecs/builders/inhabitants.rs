use crate::{
    ecs::{
        components::{
            inventory::Inventory, network::NetworkData, position::Position, task::TaskList,
        },
        storage::{Entity, World},
    },
    utils::orientation::RelativeOrientation,
};

pub fn build_inhabitant(
    x: u32,
    y: u32,
    orientation: RelativeOrientation,
    world: &mut World,
    network_data: NetworkData,
) -> Entity {
    let new_inhabitant = world.spawn();
    world.add_component(new_inhabitant, TaskList::default());
    world.add_component(new_inhabitant, Position::new());
    world.add_component(new_inhabitant, Inventory::new());
    world.add_component(new_inhabitant, RelativeOrientation::Forward);
    world.add_component(new_inhabitant, network_data);

    let position = world.get_component_mut::<Position>(new_inhabitant).unwrap();
    position.x = x;
    position.y = y;

    let mut _orientation = world
        .get_component_mut::<RelativeOrientation>(new_inhabitant)
        .unwrap();
    *_orientation = orientation;

    new_inhabitant
}
