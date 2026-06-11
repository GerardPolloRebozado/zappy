use crate::ecs::{
    components::{inventory::Inventory, position::Position, tile::Tile, terrain_type::TerrainType},
    storage::{Entity, World},
};

pub fn build_tile(pos: Position, world: &mut World, terrain: TerrainType) -> Entity {
    let entity = world.spawn();
    world.add_component(entity, Tile);
    world.add_component(entity, Inventory::new());
    world.add_component(entity, pos);
    world.add_component(entity, terrain);
    entity
}
