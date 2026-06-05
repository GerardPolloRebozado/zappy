use crate::ecs::components::network::NetworkData;
use crate::ecs::components::task::TaskType;
use crate::ecs::storage::Entity;
use crate::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use crate::server::Server;
use crate::server::commands::queue_task;

pub fn handle_ai_command(server: &mut Server, entity: Entity, request: Request) {
    match request.command {
        Command::Forward => queue_task(server, entity, TaskType::Forward),
        Command::Right => queue_task(server, entity, TaskType::TurnRight),
        Command::Left => queue_task(server, entity, TaskType::TurnLeft),
        Command::Look => queue_task(server, entity, TaskType::Look),
        Command::Inventory => queue_task(server, entity, TaskType::Inventory),
        Command::Broadcast(text) => queue_task(server, entity, TaskType::BroadcastText(text)),
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
                .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
        }
    }
}
