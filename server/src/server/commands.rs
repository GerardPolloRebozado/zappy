use crate::ecs::builders::inhabitants::build_inhabitant;
use crate::ecs::components::task::{TASK_NOT_STARTED, Task, TaskList, TaskType};
use crate::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use crate::server::Server;
use crate::server::client::ClientState;
use crate::server::map_size;

pub fn queue_task(server: &mut Server, client_uuid: &str, task_type: TaskType) {
    let client = match server.clients.get(client_uuid) {
        Some(c) => c,
        None => return,
    };

    let entity = match client.entity {
        Some(e) => e,
        None => return,
    };

    let task_list = match server.world.get_component_mut::<TaskList>(entity) {
        Some(tl) => tl,
        None => return,
    };

    if task_list.vector.len() >= 10 {
        return;
    }

    task_list.vector.push(Task {
        task_type,
        finish_on: TASK_NOT_STARTED,
    });
}

pub fn handle_request(server: &mut Server, client_uuid: &str, request: Request) {
    let client_state = match server.clients.get(client_uuid) {
        Some(c) => c.state.clone(),
        None => return,
    };

    if client_state == ClientState::WaitingForTeamName {
        handle_auth_request(server, client_uuid, request);
        return;
    }

    match request.command {
        Command::Forward => queue_task(server, client_uuid, TaskType::Forward),
        Command::Right => queue_task(server, client_uuid, TaskType::TurnRight),
        Command::Left => queue_task(server, client_uuid, TaskType::TurnLeft),
        Command::Look => queue_task(server, client_uuid, TaskType::Look),
        Command::Inventory => queue_task(server, client_uuid, TaskType::Inventory),
        Command::Broadcast(_) => queue_task(server, client_uuid, TaskType::BroadcastText),
        Command::ConnectNbr => {
            let client = match server.clients.get_mut(client_uuid) {
                Some(c) => c,
                None => return,
            };
            client.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some("1".to_string()),
            ));
        }
        Command::Fork => queue_task(server, client_uuid, TaskType::Fork),
        Command::Eject => queue_task(server, client_uuid, TaskType::Eject),
        Command::Take(_) => queue_task(server, client_uuid, TaskType::Take),
        Command::Set(_) => queue_task(server, client_uuid, TaskType::Drop),
        Command::Incantation => queue_task(server, client_uuid, TaskType::Incantation),

        Command::Msz => {
            let width = server.mapSize.width;
            let height = server.mapSize.height;
            let client = match server.clients.get_mut(client_uuid) {
                Some(c) => c,
                None => return,
            };
            client.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(format!("msz {} {}", width, height)),
            ));
        }

        Command::Bct(x, y) => {
            let data = match map_size::get_tile_content(&server.world, x, y) {
                Some(d) => d,
                None => return,
            };
            let client = match server.clients.get_mut(client_uuid) {
                Some(c) => c,
                None => return,
            };
            client.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(data),
            ));
        }

        Command::Mct => {
            println!("Protocol: Received mct from {}", client_uuid);
            let mut responses = Vec::new();
            let width = server.mapSize.width;
            let height = server.mapSize.height;
            for y in 0..height {
                for x in 0..width {
                    if let Some(data) = map_size::get_tile_content(&server.world, x, y) {
                        responses.push(data);
                    }
                }
            }
            let client = match server.clients.get_mut(client_uuid) {
                Some(c) => c,
                None => return,
            };
            for r in responses {
                client
                    .pending_responses
                    .push(Response::new(ResponseCode::Status(StatusCode::Ok), Some(r)));
            }
        }

        Command::Unknown(_) => {
            let client = match server.clients.get_mut(client_uuid) {
                Some(c) => c,
                None => return,
            };
            client.pending_responses.push(Response {
                code: ResponseCode::Status(StatusCode::Ko),
                data: None,
            });
        }

        _ => {
            let client = match server.clients.get_mut(client_uuid) {
                Some(c) => c,
                None => return,
            };
            client
                .pending_responses
                .push(Response::new(ResponseCode::Status(StatusCode::Ok), None));
        }
    }
}

use crate::utils::orientation::RelativeOrientation;

pub fn handle_auth_request(server: &mut Server, client_uuid: &str, request: Request) {
    let client = match server.clients.get_mut(client_uuid) {
        Some(c) => c,
        None => return,
    };

    let team_name = match request.command {
        Command::Unknown(team_name) => team_name,
        _ => {
            client
                .pending_responses
                .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
            return;
        }
    };

    if team_name == "GRAPHIC" {
        client.state = ClientState::AuthenticatedGUI;
        return;
    }

    client.state = ClientState::AuthenticatedAI(team_name);

    client.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some("1".to_string()),
    ));

    let entity = build_inhabitant(0, 0, RelativeOrientation::Forward, &mut server.world);
    client.entity = Some(entity);
    if let Some(task_list) = server.world.get_component_mut::<TaskList>(entity) {
        task_list.client_uuid = Some(client_uuid.to_string());
    }

    let width = server.mapSize.width;
    let height = server.mapSize.height;
    client.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some(format!("{} {}", width, height)),
    ));
}
