use crate::ecs::components::inhabitant::Inhabitant;
use crate::ecs::components::position::Position;
use crate::ecs::storage::{Entity, World};
use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};
use crate::utils::orientation::RelativeOrientation;
use log::info;

/// Runs the AI `Forward` task for `entity`.
///
/// Updates [`Position`] from [`RelativeOrientation`] on the same entity and returns
/// `ok` plus a [`ServerEvent::PlayerPosition`] for GUI fan-out.
pub fn execute_forward(world: &mut World, entity: Entity) -> (Response, Option<ServerEvent>) {
    info!("Moving forward entity: {}", entity.id());
    let map_width = world.map_size.width;
    let map_height = world.map_size.height;
    let orientation = world.get_component::<RelativeOrientation>(entity).copied();
    if let Some(pos) = world.get_component_mut::<Position>(entity)
        && let Some(ori) = orientation
    {
        pos.move_forward(ori, map_width, map_height);
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
    fn execute_forward_moves_north() {
        let (mut world, entity) = setup_inhabitant(5, 5, RelativeOrientation::Forward, 10, 10);
        let (response, _) = execute_forward(&mut world, entity);
        assert_ok(response);
        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (5, 4));
    }

    #[test]
    fn execute_forward_returns_player_position_event() {
        let (mut world, entity) = setup_inhabitant(5, 5, RelativeOrientation::Forward, 10, 10);
        let (response, event) = execute_forward(&mut world, entity);
        assert_ok(response);
        assert!(matches!(
            event,
            Some(ServerEvent::PlayerPosition {
                player_id,
                x: 5,
                y: 4,
                orientation: RelativeOrientation::Forward,
            }) if player_id == entity.id()
        ));
    }

    #[test]
    fn execute_forward_moves_east() {
        let (mut world, entity) = setup_inhabitant(5, 5, RelativeOrientation::Right, 10, 10);
        execute_forward(&mut world, entity);
        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (6, 5));
    }

    #[test]
    fn execute_forward_wraps() {
        let (mut world, entity) = setup_inhabitant(3, 0, RelativeOrientation::Forward, 10, 10);
        execute_forward(&mut world, entity);
        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (3, 9));
    }

    #[test]
    fn execute_forward_without_orientation() {
        let mut world = World::default();
        world.map_size.width = 10;
        world.map_size.height = 10;
        let entity = world.spawn();
        world.add_component(entity, Position { x: 5, y: 5 });

        execute_forward(&mut world, entity);

        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (5, 5));
    }
}
