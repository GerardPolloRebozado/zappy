use crate::ecs::{
    components::position::Position,
    storage::{Entity, World},
};

/// Marker component to identify a Tile entity.
#[derive(Debug, Default)]
pub struct Tile;

impl Tile {
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
}

#[cfg(test)]
mod tests {
    use crate::ecs::builders::tile::build_tile;

    use super::*;

    #[test]
    fn find_existing_tile() {
        let mut world = World::default();
        let tile_entity = build_tile(Position { x: 0, y: 0 }, &mut world);

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
