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
