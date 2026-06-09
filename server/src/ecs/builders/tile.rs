use crate::ecs::{
    components::{inventory::Inventory, position::Position, tile::Tile},
    storage::{Entity, World},
};

pub fn build_tile(pos: Position, world: &mut World) -> Entity {
    let entity = world.spawn();
    world.add_component(entity, Tile);
    world.add_component(entity, Inventory::new());
    world.add_component(entity, pos);
    entity
}
