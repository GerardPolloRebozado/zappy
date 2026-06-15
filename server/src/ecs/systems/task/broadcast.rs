use crate::ecs::components::inhabitant::Inhabitant;
use crate::ecs::storage::{Entity, World};
use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};

/// Runs the AI `BroadcastText` task for `entity`.
///
/// Does not mutate the world: it snapshots the broadcaster's position for a
/// [`ServerEvent::Message`] that [`super::broadcast_event`] fans out to other clients.
pub fn execute_broadcast(
    world: &World,
    entity: Entity,
    text: &str,
) -> (Response, Option<ServerEvent>) {
    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), None);
    let event =
        Inhabitant::get(entity, world).map(|broadcaster| ServerEvent::message(&broadcaster, text));
    (ok, event)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::components::network::{MockSocket, NetworkData};
    use crate::ecs::storage::World;
    use crate::utils::orientation::RelativeOrientation;

    #[test]
    fn execute_broadcast_returns_message_event() {
        let mut world = World::default();
        world.map_size.width = 10;
        world.map_size.height = 10;

        let (mock_socket, _) = MockSocket::new(Vec::from(""));
        let network_data = NetworkData::new(mock_socket);
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, 2, 3, RelativeOrientation::Forward, &mut world);
        world.add_component(entity, network_data);

        let (response, event) = execute_broadcast(&world, entity, "hi");
        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));
        assert!(response.data.is_none());
        assert!(matches!(
            event,
            Some(ServerEvent::Message {
                message,
                x: 2,
                y: 3,
                ..
            }) if message == "hi"
        ));
    }
}
