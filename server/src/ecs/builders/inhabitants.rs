use crate::ecs::{
    components::{inventory::Inventory, position::Position, task::TaskList},
    storage::{Entity, World},
};

pub fn build_inhabitant(world: &mut World) -> Entity {
    let new_inhabitant = world.spawn();
    world.add_component(new_inhabitant, TaskList::default());
    world.add_component(new_inhabitant, Position::new());
    world.add_component(new_inhabitant, Inventory::new());
    new_inhabitant
}
