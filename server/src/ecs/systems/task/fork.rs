use crate::ecs::components::egg::Egg;
use crate::ecs::components::position::Position;
use crate::ecs::components::team::Team;
use crate::ecs::storage::{Entity, World};
use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};

/// Runs the AI `Fork` task for an entity.
///
/// Spawns a new egg entity with the executor's position and team.
/// This allows a new client to connect to the game server
///
/// Note: The start of the fork task (sending the `pfk` GUI event)
/// is managed upstream in the main task dispatcher (`any_finished_task`),
/// whereas this function handles the completion: actually spawning the egg
/// and emitting the `enw` GUI event via `ServerEvent::EggLaid`
pub fn execute_fork(world: &mut World, entity: Entity) -> (Response, Option<ServerEvent>) {
    let position = world.get_component::<Position>(entity).cloned();
    let team = world.get_component::<Team>(entity).cloned();

    if let (Some(pos), Some(Team::AuthenticatedAI(team_name))) = (position, team) {
        let egg_entity = world.spawn();
        world.add_component(egg_entity, pos.clone());
        world.add_component(
            egg_entity,
            Egg {
                team: team_name,
                player_id: entity.id(),
            },
        );

        (
            Response::new(ResponseCode::Status(StatusCode::Ok), None),
            Some(ServerEvent::EggLaid {
                egg_id: egg_entity.id(),
                player_id: entity.id(),
                x: pos.x,
                y: pos.y,
            }),
        )
    } else {
        (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::utils::orientation::RelativeOrientation;

    #[test]
    fn test_execute_fork() {
        let mut world = World::default();
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, 5, 5, RelativeOrientation::Forward, &mut world);
        world.add_component(entity, Team::AuthenticatedAI("team1".to_string()));

        let (response, event) = execute_fork(&mut world, entity);

        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));

        let egg_id = if let Some(ServerEvent::EggLaid { egg_id, .. }) = event {
            egg_id
        } else {
            panic!("Expected EggLaid event");
        };

        let egg_storage = world.get_storage::<Egg>().unwrap();
        let mut found_egg_entity = None;
        for (e, _) in egg_storage.iter() {
            if e.id() == egg_id {
                found_egg_entity = Some(*e);
            }
        }

        let egg_entity = found_egg_entity.unwrap();
        let pos = world.get_component::<Position>(egg_entity).unwrap();
        assert_eq!(pos.x, 5);
        assert_eq!(pos.y, 5);

        let egg = world.get_component::<Egg>(egg_entity).unwrap();
        assert_eq!(egg.team, "team1");
        assert_eq!(egg.player_id, entity.id());
    }

    #[test]
    fn test_execute_fork_no_team() {
        let mut world = World::default();
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, 5, 5, RelativeOrientation::Forward, &mut world);
        // Intentionally missing Team component

        let (response, event) = execute_fork(&mut world, entity);

        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ko));
        assert!(event.is_none());
    }

    #[test]
    fn test_execute_fork_no_position() {
        let mut world = World::default();
        let entity = world.spawn();
        world.add_component(entity, Team::AuthenticatedAI("team1".to_string()));
        // Intentionally missing Position component

        let (response, event) = execute_fork(&mut world, entity);

        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ko));
        assert!(event.is_none());
    }
}
