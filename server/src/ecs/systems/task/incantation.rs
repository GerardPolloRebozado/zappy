use crate::ecs::components::inventory::Inventory;
use crate::ecs::components::level::Level;
use crate::ecs::components::network::NetworkData;
use crate::ecs::components::position::Position;
use crate::ecs::components::resource::Resource;
use crate::ecs::components::task::TaskList;
use crate::ecs::components::tile::Tile;
use crate::ecs::storage::{Entity, World};
use crate::ecs::systems::task::broadcast_event;
use crate::protocol::{Response, ResponseCode, ServerEvent, StatusCode};

/// Processes incantations that have just started.
///
/// It checks if the prerequisites for the incantation are met. If they are, it broadcasts an
/// `Elevation underway` message to all participating players, as well as a `pic` event to GUI clients.
/// If the prerequisites are not met, the incantation is immediately aborted and the initiating
/// player receives a `ko` response.
pub fn process_started_incantations(world: &mut World, started_incantations: Vec<Entity>) {
    for entity in started_incantations {
        if let Some((_, participants, level)) = check_incantation_prerequisites(world, entity) {
            let pos = world.get_component::<Position>(entity).unwrap().clone();
            let event = ServerEvent::start_incantation(
                pos.x,
                pos.y,
                level,
                participants.iter().map(|e| e.id()).collect(),
            );
            broadcast_event(world, event);

            for participant in participants {
                if let Some(nd) = world.get_component_mut::<NetworkData>(participant) {
                    nd.pending_responses.push(Response::new(
                        ResponseCode::Status(StatusCode::Ok),
                        Some("Elevation underway".to_string()),
                    ));
                }
            }
        } else {
            // Prerequisites failed at the beginning
            if let Some(task_list) = world.get_component_mut::<TaskList>(entity)
                && !task_list.vector.is_empty()
            {
                task_list.vector[0].finish_on = 0;
                task_list.vector.remove(0);
            }
            if let Some(nd) = world.get_component_mut::<NetworkData>(entity) {
                nd.pending_responses
                    .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
            }
        }
    }
}

/// Checks if the prerequisites for an incantation are met for a specific player.
///
/// Validates that the tile the player is standing on contains the correct amount of required stones,
/// and that there are enough players of the same level on the same tile.
/// Returns the tile entity, a vector of participating player entities, and their current level if valid.
pub fn check_incantation_prerequisites(
    world: &mut World,
    entity: Entity,
) -> Option<(Entity, Vec<Entity>, u8)> {
    let position = world.get_component::<Position>(entity)?.clone();
    let tile_entity = Tile::find_tile_by_pos(&position, world)?;
    let level = world.get_component::<Level>(entity)?.value;

    let tile_inventory = world.get_component::<Inventory>(tile_entity)?;

    let requirements = get_requirements(level)?;
    let (
        req_players,
        req_linemate,
        req_deraumere,
        req_sibur,
        req_mendiane,
        req_phiras,
        req_thystame,
    ) = requirements;

    if tile_inventory.get_item_count(Resource::Linemate) < req_linemate
        || tile_inventory.get_item_count(Resource::Deraumere) < req_deraumere
        || tile_inventory.get_item_count(Resource::Sibur) < req_sibur
        || tile_inventory.get_item_count(Resource::Mendiane) < req_mendiane
        || tile_inventory.get_item_count(Resource::Phiras) < req_phiras
        || tile_inventory.get_item_count(Resource::Thystame) < req_thystame
    {
        return None;
    }

    let mut participating_players = Vec::new();
    if let Some(positions) = world.get_storage::<Position>() {
        for (other_entity, other_pos) in positions.iter() {
            if other_pos.x == position.x
                && other_pos.y == position.y
                && let Some(other_level) = world.get_component::<Level>(*other_entity)
                && other_level.value == level
                && world.get_component::<NetworkData>(*other_entity).is_some()
            {
                participating_players.push(*other_entity);
            }
        }
    }

    if participating_players.len() < req_players as usize {
        return None;
    }

    Some((tile_entity, participating_players, level))
}

/// Returns the required stones and players for a given level elevation.
///
/// The format returned is `(players, linemate, deraumere, sibur, mendiane, phiras, thystame)`.
/// Returns `None` if the level is invalid (e.g. already at maximum level).
pub fn get_requirements(level: u8) -> Option<(u32, u32, u32, u32, u32, u32, u32)> {
    match level {
        1 => Some((1, 1, 0, 0, 0, 0, 0)),
        2 => Some((2, 1, 1, 1, 0, 0, 0)),
        3 => Some((2, 2, 0, 1, 0, 2, 0)),
        4 => Some((4, 1, 1, 2, 0, 1, 0)),
        5 => Some((4, 1, 2, 1, 3, 0, 0)),
        6 => Some((6, 1, 2, 3, 0, 1, 0)),
        7 => Some((6, 2, 2, 2, 2, 2, 1)),
        _ => None,
    }
}

/// Executes the final step of an incantation task.
///
/// Validates the prerequisites one last time. If successful, consumes the required stones from the tile,
/// increments the level of all participating players, and returns the appropriate server responses and events.
/// If the prerequisites are no longer met (e.g. stones were removed during the ritual), it returns `ko`.
pub fn execute_incantation(world: &mut World, entity: Entity) -> (Response, Option<ServerEvent>) {
    let ko = Response::new(ResponseCode::Status(StatusCode::Ko), None);

    // Validate prerequisites again
    let (tile_entity, participants, level) = match check_incantation_prerequisites(world, entity) {
        Some(res) => res,
        None => {
            let pos = world.get_component::<Position>(entity).unwrap().clone();
            let event = ServerEvent::EndIncantation {
                x: pos.x,
                y: pos.y,
                result: 0,
            };
            return (ko, Some(event));
        }
    };

    let requirements = get_requirements(level).unwrap();
    let (_, req_linemate, req_deraumere, req_sibur, req_mendiane, req_phiras, req_thystame) =
        requirements;

    // Remove stones
    if let Some(tile_inv) = world.get_component_mut::<Inventory>(tile_entity) {
        tile_inv.remove_item(Resource::Linemate, req_linemate);
        tile_inv.remove_item(Resource::Deraumere, req_deraumere);
        tile_inv.remove_item(Resource::Sibur, req_sibur);
        tile_inv.remove_item(Resource::Mendiane, req_mendiane);
        tile_inv.remove_item(Resource::Phiras, req_phiras);
        tile_inv.remove_item(Resource::Thystame, req_thystame);
    }

    let new_level = level + 1;
    let ok_resp_data = format!("Current level: {}", new_level);

    // Increase level of participants
    for participant in &participants {
        if let Some(lvl) = world.get_component_mut::<Level>(*participant) {
            lvl.value = new_level;
        } else {
            println!("No level");
            panic!("No level");
        }

        // Push response to other participants. The initiator's response is returned.
        if *participant != entity
            && let Some(nd) = world.get_component_mut::<NetworkData>(*participant)
        {
            nd.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(ok_resp_data.clone()),
            ));
        }
    }

    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), Some(ok_resp_data));

    let pos = world.get_component::<Position>(entity).unwrap().clone();
    let event = ServerEvent::EndIncantation {
        x: pos.x,
        y: pos.y,
        result: 1, // 1 for success
    };

    (ok, Some(event))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::builders::tile::build_tile;
    use crate::ecs::components::network::{MockSocket, NetworkData};
    use crate::ecs::components::terrain_type::TerrainType;
    use crate::utils::orientation::RelativeOrientation;

    fn setup_world_with_tile() -> (World, Entity) {
        let mut world = World::default();
        let tile = build_tile(Position { x: 0, y: 0 }, &mut world, TerrainType::Grass);
        (world, tile)
    }

    fn spawn_player(world: &mut World, level: u8) -> Entity {
        let (mock_socket, _) = MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        let entity = world.spawn();
        let player =
            build_inhabitant_with_entity(entity, 0, 0, RelativeOrientation::Forward, world);
        world.add_component(entity, network_data);
        world.get_component_mut::<Level>(player).unwrap().value = level;
        player
    }

    #[test]
    fn test_get_requirements() {
        assert_eq!(get_requirements(1), Some((1, 1, 0, 0, 0, 0, 0)));
        assert_eq!(get_requirements(2), Some((2, 1, 1, 1, 0, 0, 0)));
        assert_eq!(get_requirements(8), None);
    }

    #[test]
    fn test_check_prerequisites_fails_not_enough_stones() {
        let (mut world, _tile) = setup_world_with_tile();
        let player = spawn_player(&mut world, 1);

        assert!(check_incantation_prerequisites(&mut world, player).is_none());
    }

    #[test]
    fn test_check_prerequisites_success() {
        let (mut world, tile) = setup_world_with_tile();
        let player = spawn_player(&mut world, 1);

        world
            .get_component_mut::<Inventory>(tile)
            .unwrap()
            .add_item(Resource::Linemate, 1);

        let res = check_incantation_prerequisites(&mut world, player);
        assert!(res.is_some());
        let (found_tile, participants, level) = res.unwrap();
        assert_eq!(found_tile, tile);
        assert_eq!(participants.len(), 1);
        assert_eq!(level, 1);
    }

    #[test]
    fn test_execute_incantation_success() {
        let (mut world, tile) = setup_world_with_tile();
        let player = spawn_player(&mut world, 1);

        world
            .get_component_mut::<Inventory>(tile)
            .unwrap()
            .add_item(Resource::Linemate, 1);

        let (response, _event) = execute_incantation(&mut world, player);

        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));
        assert_eq!(response.data, Some("Current level: 2".to_string()));

        // Stones should be removed
        assert_eq!(
            world
                .get_component::<Inventory>(tile)
                .unwrap()
                .get_item_count(Resource::Linemate),
            0
        );

        // Level should be 2
        assert_eq!(world.get_component::<Level>(player).unwrap().value, 2);
    }
}
