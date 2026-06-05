use crate::ecs::components::network::{ClientState, NetworkData};
use crate::ecs::components::task::{TASK_NOT_STARTED, Task, TaskList, TaskType};
use crate::ecs::storage::Entity;
use crate::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use crate::server::Server;
use crate::server::commands::ai::handle_ai_command;
use crate::server::commands::gui::handle_gui_command;

pub mod ai;
pub mod gui;

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
    let network_data = server.world.get_component_mut::<NetworkData>(entity);
    if network_data.is_none() {
        return;
    }
    let network_data = network_data.unwrap();

    match network_data.state.clone() {
        ClientState::AuthenticatedAI(_) => handle_ai_command(server, entity, request),
        ClientState::WaitingForTeamName => handle_auth_request(server, entity, request),
        ClientState::AuthenticatedGUI => handle_gui_command(server, entity, request),
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
