//! Late-join GUI catch-up: ECS snapshot → synthetic protocol lines.
//!
//! When a GUI client authenticates with `GRAPHIC` after AI players have
//! already spawned, this module replays the current world state as protocol
//! notifications (`pnw` for players, `enw` for eggs) so the GUI starts
//! with a complete picture.  Called from [`super::handle_auth_request`].

use crate::ecs::components::egg::Egg;
use crate::ecs::components::inhabitant::Inhabitant;
use crate::ecs::components::inhabitant_tag::InhabitantTag;
use crate::ecs::components::network::NetworkData;
use crate::ecs::components::position::Position;
use crate::ecs::components::team::Team;
use crate::ecs::storage::{Entity, World};
use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};

/// Converts each [`ServerEvent`] to its GUI string and pushes it as an `Ok`
/// response into the GUI client's [`NetworkData::pending_responses`].
pub fn push_gui_event_lines(world: &mut World, gui_entity: Entity, events: Vec<ServerEvent>) {
    let lines: Vec<String> = events.iter().filter_map(|e| e.to_gui_string()).collect();

    let network_data = match world.get_component_mut::<NetworkData>(gui_entity) {
        Some(nd) => nd,
        None => return,
    };

    for line in lines {
        network_data.pending_responses.push(Response::new(
            ResponseCode::Status(StatusCode::Ok),
            Some(line),
        ));
    }
}

/// Builds a [`ServerEvent::NewPlayer`] for every `InhabitantTag` entity
/// whose [`Team`] is `AuthenticatedAI`.  Entities missing required
/// components are silently skipped.
pub fn collect_player_sync_events(world: &World) -> Vec<ServerEvent> {
    let inhabitant_storage = match world.get_storage::<InhabitantTag>() {
        Some(s) => s,
        None => return Vec::new(),
    };

    let mut events = Vec::new();

    for (entity, _) in inhabitant_storage.iter() {
        let team_name = match world.get_component::<Team>(*entity) {
            Some(Team::AuthenticatedAI(name)) => name.clone(),
            _ => continue,
        };

        let inhabitant = match Inhabitant::get(*entity, world) {
            Some(i) => i,
            None => continue,
        };

        let level = inhabitant.level().value;
        events.push(ServerEvent::new_player(&inhabitant, level, team_name));
    }

    events
}

/// Builds a [`ServerEvent::EggLaid`] for every [`Egg`] entity on the map.
pub fn collect_egg_sync_events(world: &World) -> Vec<ServerEvent> {
    let egg_storage = match world.get_storage::<Egg>() {
        Some(s) => s,
        None => return Vec::new(),
    };

    let mut events = Vec::new();

    for (entity, egg) in egg_storage.iter() {
        let position = match world.get_component::<Position>(*entity) {
            Some(p) => p,
            None => continue,
        };

        events.push(ServerEvent::EggLaid {
            egg_id: entity.id(),
            player_id: egg.player_id,
            x: position.x,
            y: position.y,
        });
    }

    events
}

/// Replays `pnw` lines to a GUI client for all existing AI players.
pub fn sync_players_to_gui(world: &mut World, gui_entity: Entity) {
    let events = collect_player_sync_events(world);
    push_gui_event_lines(world, gui_entity, events);
}

/// Replays `enw` lines to a GUI client for all existing eggs.
pub fn sync_eggs_to_gui(world: &mut World, gui_entity: Entity) {
    let events = collect_egg_sync_events(world);
    push_gui_event_lines(world, gui_entity, events);
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::components::network::MockSocket;
    use crate::ecs::storage::World;
    use crate::utils::orientation::RelativeOrientation;

    #[test]
    fn test_collect_player_sync_events_empty_world() {
        let world = World::default();
        let events = collect_player_sync_events(&world);
        assert!(events.is_empty());
    }

    #[test]
    fn test_collect_player_sync_events_with_ai() {
        let mut world = World::default();
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, 3, 7, RelativeOrientation::Forward, &mut world);
        *world.get_component_mut::<Team>(entity).unwrap() =
            Team::AuthenticatedAI("TeamA".to_string());

        let events = collect_player_sync_events(&world);
        assert_eq!(events.len(), 1);

        match &events[0] {
            ServerEvent::NewPlayer {
                player_id,
                x,
                y,
                level,
                team,
                ..
            } => {
                assert_eq!(*player_id, entity.id());
                assert_eq!(*x, 3);
                assert_eq!(*y, 7);
                assert_eq!(*level, 1);
                assert_eq!(team, "TeamA");
            }
            other => panic!("Expected NewPlayer, got {:?}", other),
        }
    }

    #[test]
    fn test_collect_player_sync_events_skips_non_ai() {
        let mut world = World::default();

        let gui = world.spawn();
        build_inhabitant_with_entity(gui, 0, 0, RelativeOrientation::Forward, &mut world);
        *world.get_component_mut::<Team>(gui).unwrap() = Team::AuthenticatedGUI;

        let waiting = world.spawn();
        build_inhabitant_with_entity(waiting, 1, 1, RelativeOrientation::Forward, &mut world);

        let events = collect_player_sync_events(&world);
        assert!(events.is_empty());
    }

    #[test]
    fn test_collect_egg_sync_events() {
        let mut world = World::default();
        let egg = world.spawn();
        world.add_component(
            egg,
            Egg {
                team: "team1".to_string(),
                player_id: 0,
            },
        );
        world.add_component(egg, Position { x: 5, y: 9 });

        let events = collect_egg_sync_events(&world);
        assert_eq!(events.len(), 1);

        match &events[0] {
            ServerEvent::EggLaid {
                egg_id,
                player_id,
                x,
                y,
            } => {
                assert_eq!(*egg_id, egg.id());
                assert_eq!(*player_id, 0);
                assert_eq!(*x, 5);
                assert_eq!(*y, 9);
            }
            other => panic!("Expected EggLaid, got {:?}", other),
        }
    }

    #[test]
    fn test_collect_egg_sync_events_preserves_player_id() {
        let mut world = World::default();
        let egg = world.spawn();
        world.add_component(
            egg,
            Egg {
                team: "team1".to_string(),
                player_id: 99,
            },
        );
        world.add_component(egg, Position { x: 2, y: 3 });

        let events = collect_egg_sync_events(&world);
        assert_eq!(events.len(), 1);

        match &events[0] {
            ServerEvent::EggLaid { player_id, .. } => assert_eq!(*player_id, 99),
            other => panic!("Expected EggLaid, got {:?}", other),
        }
    }

    #[test]
    fn test_collect_egg_sync_events_empty_world() {
        let world = World::default();
        let events = collect_egg_sync_events(&world);
        assert!(events.is_empty());
    }

    #[test]
    fn test_sync_players_to_gui_pushes_pnw() {
        let mut world = World::default();

        let ai = world.spawn();
        build_inhabitant_with_entity(ai, 2, 4, RelativeOrientation::Forward, &mut world);
        *world.get_component_mut::<Team>(ai).unwrap() = Team::AuthenticatedAI("TeamB".to_string());

        let gui = world.spawn();
        let (mock_socket, _) = MockSocket::new(vec![]);
        world.add_component(gui, NetworkData::new(mock_socket));

        sync_players_to_gui(&mut world, gui);

        let nd = world.get_component::<NetworkData>(gui).unwrap();
        assert_eq!(nd.pending_responses.len(), 1);
        let data = nd.pending_responses[0].data.as_ref().unwrap();
        assert!(data.starts_with("pnw"), "expected pnw line, got: {}", data);
    }

    #[test]
    fn test_sync_eggs_to_gui_pushes_enw() {
        let mut world = World::default();

        let egg = world.spawn();
        world.add_component(
            egg,
            Egg {
                team: "team2".to_string(),
                player_id: 42,
            },
        );
        world.add_component(egg, Position { x: 1, y: 2 });

        let gui = world.spawn();
        let (mock_socket, _) = MockSocket::new(vec![]);
        world.add_component(gui, NetworkData::new(mock_socket));

        sync_eggs_to_gui(&mut world, gui);

        let nd = world.get_component::<NetworkData>(gui).unwrap();
        assert_eq!(nd.pending_responses.len(), 1);
        let data = nd.pending_responses[0].data.as_ref().unwrap();
        assert!(data.starts_with("enw"), "expected enw line, got: {}", data);
    }
}
