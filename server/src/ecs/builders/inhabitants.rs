use crate::ecs::{
    components::{
        inventory::Inventory, orientation::Orientation, position::Position, task::TaskList,
    },
    storage::{Entity, World},
};

pub fn build_inhabitant(x: u32, y: u32, orientation: u8, world: &mut World) -> Entity {
    let new_inhabitant = world.spawn();
    world.add_component(new_inhabitant, TaskList::default());
    world.add_component(new_inhabitant, Position::new());
    world.add_component(new_inhabitant, Inventory::new());
    world.add_component(new_inhabitant, Orientation::default());

    let position = world.get_component_mut::<Position>(new_inhabitant).unwrap();
    position.x = x;
    position.y = y;

    let _orientation = world
        .get_component_mut::<Orientation>(new_inhabitant)
        .unwrap();
    _orientation.orientation = orientation;

    new_inhabitant
}
