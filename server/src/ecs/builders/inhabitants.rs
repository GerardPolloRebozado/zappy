use crate::ecs::{
    components::{inventory::Inventory, position::Position, task::TaskList},
    storage::{Entity, World},
};

pub fn build_inhabitant(world: &mut World) -> Entity {
    let new_player = world.spawn();
    world.add_component(new_player, TaskList::default());
    world.add_component(new_player, Position::new());
    world.add_component(new_player, Inventory::new());
    new_player
}
