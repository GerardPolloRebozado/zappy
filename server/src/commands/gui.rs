use crate::commands::ppo;
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

        Command::Tna => {
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            for team in server.team_names.iter() {
                network_data.pending_responses.push(Response::new(
                    ResponseCode::Status(StatusCode::Ok),
                    Some(format!("tna {}", team)),
                ));
            }
        }

        // Returns the current server time unit (frequency).
        // Response: `sgt T` where T is the current frequency.
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

        // Sets a new server time unit (frequency).
        // Updates `world.freq` which affects all time-based calculations
        // (task durations, food consumption, resource spawning).
        // Response: `sst T` where T is the newly set frequency.
        Command::Sst(new_freq) => {
            server.world.freq = new_freq as u64;
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();
            network_data.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(format!("sst {}", new_freq)),
            ));
        }

        Command::Ppo(id) => {
            let mut line = None;

            if let Some(player_id) = ppo::parse_player_id(&id)
                && let Some(player_entity) = ppo::find_inhabitant(&server.world, player_id)
            {
                line = ppo::build_ppo_line(&server.world, player_id, player_entity);
            }

            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();

            if let Some(line) = line {
                network_data.pending_responses.push(Response::new(
                    ResponseCode::Status(StatusCode::Ok),
                    Some(line),
                ));
            } else {
                network_data
                    .pending_responses
                    .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
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
                .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::components::network::MockSocket;
    use crate::ecs::storage::World;

    fn create_test_server(freq: u32) -> Server {
        Server {
            listener: Some(std::net::TcpListener::bind("127.0.0.1:0").unwrap()),
            _users: std::collections::HashMap::new(),
            _freq: freq,
            game_start: 0,
            world: World::new(
                crate::ecs::map_size::MapSize {
                    width: 10,
                    height: 10,
                },
                freq as u64,
                0,
            ),
            clients_nb: 1,
            team_names: vec!["TeamA".to_string(), "TeamB".to_string()],
        }
    }

    fn spawn_gui_entity(server: &mut Server) -> Entity {
        let entity = server.world.spawn();
        let (mock_socket, _) = MockSocket::new(Vec::from(""));
        let network_data = NetworkData::new(mock_socket);
        server.world.add_component(entity, network_data);
        entity
    }

    #[test]
    fn test_sgt_command() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Sgt,
            },
        );

        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "sgt 100"
        );
    }

    #[test]
    fn test_sst_command() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Sst(200),
            },
        );

        assert_eq!(server.world.freq, 200);
        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "sst 200"
        );
    }

    #[test]
    fn test_sst_then_sgt() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Sst(50),
            },
        );
        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Sgt,
            },
        );

        assert_eq!(server.world.freq, 50);
        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 2);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "sst 50"
        );
        assert_eq!(
            network_data.pending_responses[1].data.as_ref().unwrap(),
            "sgt 50"
        );
    }

    #[test]
    fn test_tna_command() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Tna,
            },
        );

        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 2);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "tna TeamA"
        );
        assert_eq!(
            network_data.pending_responses[1].data.as_ref().unwrap(),
            "tna TeamB"
        );
    }
}
