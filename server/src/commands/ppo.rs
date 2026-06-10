use crate::ecs::components::life::Life;
use crate::ecs::components::position::Position;
use crate::ecs::storage::{Entity, World};
use crate::utils::orientation::RelativeOrientation;

/// Parses `#n` or `n` from a GUI `ppo` poll argument.
pub fn parse_player_id(id: &str) -> Option<u32> {
    id.strip_prefix('#').unwrap_or(id).parse().ok()
}

/// Finds an inhabitant entity by its protocol player id.
pub fn find_inhabitant(world: &World, player_id: u32) -> Option<Entity> {
    let life_storage = world.get_storage::<Life>()?;

    for (candidate, _) in life_storage.iter() {
        if candidate.id() == player_id {
            return Some(*candidate);
        }
    }

    None
}

/// Builds the `ppo #n X Y O` reply line for a known inhabitant.
pub fn build_ppo_line(world: &World, player_id: u32, player_entity: Entity) -> Option<String> {
    let position = world.get_component::<Position>(player_entity)?;
    let orientation = world.get_component::<RelativeOrientation>(player_entity)?;
    let (x, y) = position.protocol_xy();

    Some(format!(
        "ppo #{player_id} {x} {y} {}",
        orientation.as_protocol_k()
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::storage::World;

    #[test]
    fn parse_player_id_accepts_hash_prefix() {
        assert_eq!(parse_player_id("#42"), Some(42));
        assert_eq!(parse_player_id("42"), Some(42));
        assert_eq!(parse_player_id("not-a-number"), None);
    }

    #[test]
    fn find_inhabitant_returns_matching_entity() {
        let mut world = World::default();
        let player_entity = world.spawn();
        build_inhabitant_with_entity(
            player_entity,
            0,
            0,
            RelativeOrientation::Forward,
            &mut world,
        );

        assert_eq!(
            find_inhabitant(&world, player_entity.id()),
            Some(player_entity)
        );
        assert_eq!(find_inhabitant(&world, player_entity.id() + 1), None);
    }

    #[test]
    fn build_ppo_line_formats_position_and_orientation() {
        let mut world = World::default();
        let player_entity = world.spawn();
        build_inhabitant_with_entity(
            player_entity,
            3,
            7,
            RelativeOrientation::ForwardLeft,
            &mut world,
        );

        let line = build_ppo_line(&world, player_entity.id(), player_entity).unwrap();
        assert_eq!(line, format!("ppo #{} 3 7 2", player_entity.id()));
    }
}
