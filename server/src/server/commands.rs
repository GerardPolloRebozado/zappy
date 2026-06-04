use crate::ecs::components::network::{ClientState, NetworkData};
use crate::ecs::components::task::{TASK_NOT_STARTED, Task, TaskList, TaskType};
use crate::ecs::map_size;
use crate::ecs::storage::Entity;
use crate::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use crate::server::Server;

pub fn queue_task(server: &mut Server, entity: Entity, task_type: TaskType) {
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

pub fn handle_request(server: &mut Server, entity: Entity, request: Request) {
    let width = server.world.map_size.width;
    let height = server.world.map_size.height;

    {
        let network_data = server.world.get_component_mut::<NetworkData>(entity);
        if network_data.is_none() {
            return;
        }
        let network_data = network_data.unwrap();

        if network_data.state == ClientState::WaitingForTeamName {
            handle_auth_request(server, entity, request);
            return;
        }
    }

    match request.command {
        Command::Forward => queue_task(server, entity, TaskType::Forward),
        Command::Right => queue_task(server, entity, TaskType::TurnRight),
        Command::Left => queue_task(server, entity, TaskType::TurnLeft),
        Command::Look => queue_task(server, entity, TaskType::Look),
        Command::Inventory => queue_task(server, entity, TaskType::Inventory),
        Command::Broadcast(_) => queue_task(server, entity, TaskType::BroadcastText),
        Command::ConnectNbr => {
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            network_data.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some("1".to_string()),
            ));
        }
        Command::Fork => queue_task(server, entity, TaskType::Fork),
        Command::Eject => queue_task(server, entity, TaskType::Eject),
        Command::Take(_) => queue_task(server, entity, TaskType::Take),
        Command::Set(_) => queue_task(server, entity, TaskType::Drop),
        Command::Incantation => queue_task(server, entity, TaskType::Incantation),

        Command::Msz => {
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            network_data.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(format!("msz {} {}", width, height)),
            ));
        }

        Command::Bct(x, y) => {
            let data = map_size::get_tile_content(&server.world, x, y);
            if data.is_none() {
                return;
            }
            let data = data.unwrap();
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            network_data.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(data),
            ));
        }

        Command::Mct => {
            let mut responses = Vec::new();
            let width = server.world.map_size.width;
            let height = server.world.map_size.height;
            for y in 0..height {
                for x in 0..width {
                    if let Some(data) = map_size::get_tile_content(&server.world, x, y) {
                        responses.push(data);
                    }
                }
            }
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            for r in responses {
                network_data
                    .pending_responses
                    .push(Response::new(ResponseCode::Status(StatusCode::Ok), Some(r)));
            }
        }

        Command::Unknown(_) => {
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            network_data.pending_responses.push(Response {
                code: ResponseCode::Status(StatusCode::Ko),
                data: None,
            });
        }

        _ => {
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            network_data
                .pending_responses
                .push(Response::new(ResponseCode::Status(StatusCode::Ok), None));
        }
    }
}

pub fn handle_auth_request(server: &mut Server, entity: Entity, request: Request) {
    let width = server.world.map_size.width;
    let height = server.world.map_size.height;
    let network_data = server.world.get_component_mut::<NetworkData>(entity);
    if network_data.is_none() {
        return;
    }
    let network_data = network_data.unwrap();

    let team_name = match request.command {
        Command::Unknown(team_name) => team_name,
        _ => {
            network_data
                .pending_responses
                .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
            return;
        }
    };

    if team_name == "GRAPHIC" {
        network_data.state = ClientState::AuthenticatedGUI;
        return;
    }

    network_data.state = ClientState::AuthenticatedAI(team_name);

    network_data.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some("1".to_string()),
    ));

    network_data.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some(format!("{} {}", width, height)),
    ));
}
