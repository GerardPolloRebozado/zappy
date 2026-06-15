use crate::ecs::components::inhabitant::Inhabitant;
use crate::ecs::storage::{Entity, World};
use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};
use crate::utils::orientation::RelativeOrientation;

/// Runs the AI `TurnRight` task for `entity`.
///
/// Rotates [`RelativeOrientation`] on the same entity and returns `ok` plus a
/// [`ServerEvent::PlayerPosition`] for GUI fan-out.
pub fn execute_turn_right(world: &mut World, entity: Entity) -> (Response, Option<ServerEvent>) {
    if let Some(ori) = world.get_component_mut::<RelativeOrientation>(entity) {
        *ori = ori.turn_right();
    }
    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), None);
    let event = Inhabitant::get(entity, world).map(|player| ServerEvent::player_position(&player));
    (ok, event)
}

/// Runs the AI `TurnLeft` task for `entity`.
///
/// Rotates [`RelativeOrientation`] on the same entity and returns `ok` plus a
/// [`ServerEvent::PlayerPosition`] for GUI fan-out.
pub fn execute_turn_left(world: &mut World, entity: Entity) -> (Response, Option<ServerEvent>) {
    if let Some(ori) = world.get_component_mut::<RelativeOrientation>(entity) {
        *ori = ori.turn_left();
    }
    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), None);
    let event = Inhabitant::get(entity, world).map(|player| ServerEvent::player_position(&player));
    (ok, event)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::components::network::{MockSocket, NetworkData};
    use crate::ecs::components::position::Position;
    use crate::ecs::storage::World;

    fn setup_inhabitant(
        x: u32,
        y: u32,
        orientation: RelativeOrientation,
        map_w: u32,
        map_h: u32,
    ) -> (World, Entity) {
        let mut world = World::default();
        world.map_size.width = map_w;
        world.map_size.height = map_h;

        let (mock_socket, _) = MockSocket::new(Vec::from(""));
        let network_data = NetworkData::new(mock_socket);
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, x, y, orientation, &mut world);
        world.add_component(entity, network_data);
        (world, entity)
    }

    fn assert_ok(response: Response) {
        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));
        assert!(response.data.is_none());
    }

    #[test]
    fn turn_right_rotates_orientation() {
        let (mut world, entity) = setup_inhabitant(0, 0, RelativeOrientation::Forward, 10, 10);
        super::execute_turn_right(&mut world, entity);
        let ori = world.get_component::<RelativeOrientation>(entity).unwrap();
        assert_eq!(*ori, RelativeOrientation::ForwardLeft);
    }

    #[test]
    fn turn_right_returns_player_position_event() {
        let (mut world, entity) = setup_inhabitant(2, 3, RelativeOrientation::Forward, 10, 10);
        let (response, event) = super::execute_turn_right(&mut world, entity);
        assert_ok(response);
        assert!(matches!(
            event,
            Some(ServerEvent::PlayerPosition {
                player_id,
                x: 2,
                y: 3,
                orientation: RelativeOrientation::ForwardLeft,
            }) if player_id == entity.id()
        ));
    }

    #[test]
    fn turn_left_rotates_orientation() {
        let (mut world, entity) = setup_inhabitant(0, 0, RelativeOrientation::Forward, 10, 10);
        super::execute_turn_left(&mut world, entity);
        let ori = world.get_component::<RelativeOrientation>(entity).unwrap();
        assert_eq!(*ori, RelativeOrientation::BackLeft);
    }

    #[test]
    fn turn_without_orientation() {
        let mut world = World::default();
        let entity = world.spawn();
        world.add_component(entity, Position { x: 1, y: 1 });

        let (response, _) = super::execute_turn_right(&mut world, entity);
        assert_ok(response);
    }
}
