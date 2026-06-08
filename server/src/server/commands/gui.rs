use crate::ecs::components::network::NetworkData;
use crate::ecs::map_size;
use crate::ecs::storage::Entity;
use crate::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use crate::server::Server;

pub fn handle_gui_command(server: &mut Server, entity: Entity, request: Request) {
    let width = server.world.map_size.width;
    let height = server.world.map_size.height;

    match request.command {
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

        Command::Sgt => {
            let freq = server.world.freq;
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            network_data.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(format!("sgt {}", freq)),
            ));
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
                .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
        }
    }
}
