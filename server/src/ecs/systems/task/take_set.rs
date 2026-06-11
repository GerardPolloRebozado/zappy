use log::error;

use crate::{
    ecs::{
        components::{inventory::Inventory, position::Position, resource::Resource, tile::Tile},
        storage::{Entity, World},
    },
    protocol::{
        Response, ResponseCode,
        ServerEvent::{self, ResourceCollect},
        StatusCode,
    },
};

/// Handles the "set" task, allowing a player to drop a resource on the tile they are currently on
/// It checks if the player has the resource in their inventory, removes it from there, and adds it to the tile's inventory then returns the reply and event to send to the clients
pub fn set_task(
    world: &mut World,
    entity: Entity,
    resource: &Resource,
) -> (Response, Option<ServerEvent>) {
    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), None);

    let player_position = world.get_component::<Position>(entity).unwrap().clone();
    let tile_entity = Tile::find_tile_by_pos(&player_position, world);
    if tile_entity.is_none() {
        error!(
            "Could not find tile at position ({}, {})",
            player_position.x, player_position.y
        );
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let tile_entity = tile_entity.unwrap();
    let tile_component = world.get_component_mut::<Inventory>(tile_entity);
    if tile_component.is_none() {
        error!("Could not get inventory of tile {}", tile_entity);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let tile_component = tile_component.unwrap();
    tile_component.add_item(*resource, 1);
    let player_inventory = world.get_component_mut::<Inventory>(entity);
    if player_inventory.is_none() {
        error!("Could not get inventory of player {}", entity);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let player_invetory = player_inventory.unwrap();
    let removed_resource = player_invetory.remove_item(*resource, 1);
    if !removed_resource {
        error!(
            "Could not remove resource {} from player {}",
            resource, entity
        );
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    (
        ok,
        Some(ResourceCollect {
            player_id: entity.id(),
            resource: *resource as u8,
        }),
    )
}

/// Handles the "take" task, allowing a player to take a resource from the tile they are currently on
/// It checks if the tile has the resource in its inventory, removes it from there, and adds it to the player's inventory then returns the reply and event to send to the clients
pub fn take_task(
    world: &mut World,
    entity: Entity,
    resource: &Resource,
) -> (Response, Option<ServerEvent>) {
    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), None);

    let player_position = world.get_component::<Position>(entity).unwrap().clone();
    let tile_entity = Tile::find_tile_by_pos(&player_position, world);
    if tile_entity.is_none() {
        error!(
            "Could not find tile at position ({}, {})",
            player_position.x, player_position.y
        );
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let tile_entity = tile_entity.unwrap();
    let tile_component = world.get_component_mut::<Inventory>(tile_entity);
    if tile_component.is_none() {
        error!("Could not get inventory of tile {}", tile_entity);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let tile_component = tile_component.unwrap();
    let extracted_resource = tile_component.remove_item(*resource, 1);
    if !extracted_resource {
        error!("Could not get resource {}", resource);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let player_inventory = world.get_component_mut::<Inventory>(entity);
    if player_inventory.is_none() {
        error!("Could not get inventory of player {}", entity);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let player_invetory = player_inventory.unwrap();
    player_invetory.add_item(*resource, 1);
    (
        ok,
        Some(ResourceCollect {
            player_id: entity.id(),
            resource: *resource as u8,
        }),
    )
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::builders::tile::build_tile;
    use crate::ecs::components::network::NetworkData;
    use crate::ecs::components::task::{Task, TaskList, TaskType};
    use crate::ecs::components::terrain_type::TerrainType;
    use crate::ecs::systems::task::any_finished_task;
    use crate::ecs::{self, storage};
    use crate::server::Server;

    /// gets an existing resource using tasks
    #[test]
    fn get_resource() {
        let mut server = Server::default();

        let (mock_socket, _) = crate::ecs::components::network::MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        let inhabitant = server.world.spawn();
        build_inhabitant_with_entity(
            inhabitant,
            0,
            0,
            crate::utils::orientation::RelativeOrientation::Forward,
            &mut server.world,
        );
        server.world.add_component(inhabitant, network_data);
        let tile_entity = build_tile(
            Position { x: 0, y: 0 },
            &mut server.world,
            TerrainType::Grass,
        );
        server
            .world
            .get_component_mut::<Inventory>(tile_entity)
            .unwrap()
            .add_item(Resource::Food, 1);

        let task = Task {
            task_type: TaskType::Take(Resource::Food),
            finish_on: 1,
        };
        let task_list = server
            .world
            .get_component_mut::<TaskList>(inhabitant)
            .unwrap();
        task_list.vector.push(task.clone());

        any_finished_task(&mut server.world);
        let task_list = server
            .world
            .get_component_mut::<TaskList>(inhabitant)
            .unwrap();
        assert_eq!(task_list.vector.len(), 0);

        let inhabitant_invetory = server.world.get_component::<Inventory>(inhabitant).unwrap();
        assert_eq!(inhabitant_invetory.get_item_count(Resource::Food), 11);
        let tile_inventory = server
            .world
            .get_component::<Inventory>(tile_entity)
            .unwrap();
        assert_eq!(tile_inventory.get_item_count(Resource::Food), 0);

        let network_data = server
            .world
            .get_component_mut::<NetworkData>(inhabitant)
            .unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        let response = &network_data.pending_responses[0];
        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));
    }

    #[test]
    fn set_resource() {
        let mut server = Server {
            listener: std::net::TcpListener::bind("127.0.0.1:0").unwrap(),
            _users: std::collections::HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: storage::World::new(
                ecs::map_size::MapSize {
                    width: 10,
                    height: 10,
                },
                100,
            ),
            clients_nb: 1,
            team_names: vec!["existing_team".to_string()],
        };

        let (mock_socket, _) = crate::ecs::components::network::MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        let inhabitant = server.world.spawn();
        build_inhabitant_with_entity(
            inhabitant,
            0,
            0,
            crate::utils::orientation::RelativeOrientation::Forward,
            &mut server.world,
        );
        server.world.add_component(inhabitant, network_data);
        let tile_entity = build_tile(
            Position { x: 0, y: 0 },
            &mut server.world,
            TerrainType::Grass,
        );
        server
            .world
            .get_component_mut::<Inventory>(inhabitant)
            .unwrap()
            .add_item(Resource::Food, 1);

        let task = Task {
            task_type: TaskType::Set(Resource::Food),
            finish_on: 1,
        };
        let task_list = server
            .world
            .get_component_mut::<TaskList>(inhabitant)
            .unwrap();
        task_list.vector.push(task.clone());

        any_finished_task(&mut server.world);
        let task_list = server
            .world
            .get_component_mut::<TaskList>(inhabitant)
            .unwrap();
        assert_eq!(task_list.vector.len(), 0);

        let inhabitant_invetory = server.world.get_component::<Inventory>(inhabitant).unwrap();
        assert_eq!(inhabitant_invetory.get_item_count(Resource::Food), 10);
        let tile_inventory = server
            .world
            .get_component::<Inventory>(tile_entity)
            .unwrap();
        assert_eq!(tile_inventory.get_item_count(Resource::Food), 1);
    }
}
