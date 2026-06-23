use log::error;

use crate::commands::{gev, ppo};
use crate::ecs::components::level::Level;
use crate::ecs::components::network::NetworkData;
use crate::ecs::components::tile::Tile;
use crate::ecs::map_events::map_event_from_name;
use crate::ecs::map_size;
use crate::ecs::storage::{Entity, World};
use crate::ecs::systems::map_event::activate_map_event;
use crate::protocol::{Command, Request, Response, ResponseCode, StatusCode};
use crate::server::Server;
use crate::utils::date::Date;

pub fn get_network_data(world: &mut World, entity: Entity) -> Option<&mut NetworkData> {
    world.get_component_mut::<NetworkData>(entity)
}

fn return_ko(server: &mut Server, entity: Entity) {
    if let Some(network_data) = get_network_data(&mut server.world, entity) {
        network_data.pending_responses.push(Response {
            code: ResponseCode::Status(StatusCode::Ko),
            data: None,
        });
    }
}

pub fn handle_gui_command(server: &mut Server, entity: Entity, request: Request) {
    let width = server.world.map_size.width;
    let height = server.world.map_size.height;

    match request.command {
        Command::Msz => {
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
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
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
            network_data.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(data),
            ));
        }

        Command::Mct => {
            let mut responses = Vec::new();
            let tile_storage = server.world.get_storage::<Tile>();
            if tile_storage.is_none() {
                return;
            }
            let tile_storage = tile_storage.unwrap();

            for (e, _) in tile_storage.iter() {
                responses.push(map_size::get_tile_content_by_entity(&server.world, *e));
            }
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };

            for r in responses {
                network_data
                    .pending_responses
                    .push(Response::new(ResponseCode::Status(StatusCode::Ok), Some(r)));
            }
        }

        Command::Tna => {
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
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
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
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
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
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

            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };

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

        Command::Mev(name) => {
            let width = server.world.map_size.width;
            let height = server.world.map_size.height;

            let line = if server.world.map_event.is_active() {
                None
            } else if let Some(event) = map_event_from_name(&name, width, height) {
                let now = Date::now().to_timestamp();
                activate_map_event(&mut server.world, event, now);
                Some(format!("mev {name}"))
            } else {
                None
            };

            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };

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

        Command::Gev => {
            let line = gev::build_gev_line(&server.world);
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
            network_data.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(line),
            ));
        }

        Command::Plv(id) => {
            let id_as_num = id[1..].parse::<u32>();
            if id_as_num.is_err() {
                return_ko(server, entity);
                error!("Could not parse id as number: {}", id);
                return;
            }
            let id_as_num = id_as_num.unwrap();
            let player_entity = Entity::from_id(id_as_num, &server.world);
            if player_entity.is_none() {
                return_ko(server, entity);
                error!("Could not find player entity: {}", id_as_num);
                return;
            }
            let player_entity = player_entity.unwrap();
            let level = server.world.get_component::<Level>(player_entity);
            if level.is_none() {
                return_ko(server, entity);
                error!("Could not get player level");
                return;
            }
            let level = level.unwrap().value;
            if let Some(network_data) = get_network_data(&mut server.world, entity) {
                network_data.pending_responses.push(Response {
                    code: ResponseCode::Status(StatusCode::Ok),
                    data: Some(format!("plv {} {}", id, level)),
                });
            };
        }
        Command::Unknown(_) => {
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
            network_data.pending_responses.push(Response {
                code: ResponseCode::Status(StatusCode::Ko),
                data: None,
            });
        }

        _ => {
            let Some(network_data) = get_network_data(&mut server.world, entity) else {
                return;
            };
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
    use crate::ecs::map_events::MapEvent;
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

    #[test]
    fn test_mev_triggers_solar_flare() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Mev("solar_flare".to_string()),
            },
        );

        assert!(server.world.map_event.is_active());
        assert!(matches!(
            server.world.map_event,
            MapEvent::SolarFlare { .. }
        ));
        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "mev solar_flare"
        );
    }

    #[test]
    fn test_mev_ko_when_event_active() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);
        server.world.map_event = MapEvent::new_meteor_shower();

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Mev("solar_flare".to_string()),
            },
        );

        assert!(matches!(
            server.world.map_event,
            MapEvent::MeteorShower { .. }
        ));
        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert!(network_data.pending_responses[0].data.is_none());
    }

    #[test]
    fn test_mev_ko_for_invalid_name() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Mev("unknown_event".to_string()),
            },
        );

        assert!(!server.world.map_event.is_active());
        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert!(network_data.pending_responses[0].data.is_none());
    }

    #[test]
    fn test_gev_no_active_event() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Gev,
            },
        );

        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "gev none"
        );
    }

    #[test]
    fn test_gev_with_active_event() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);
        server.world.map_event = MapEvent::new_solar_flare();

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Gev,
            },
        );

        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "gev solar_flare"
        );
    }

    #[test]
    fn test_gev_gravity_well_includes_coords() {
        let mut server = create_test_server(100);
        let entity = spawn_gui_entity(&mut server);
        server.world.map_event = MapEvent::new_gravity_well(3, 7);

        handle_gui_command(
            &mut server,
            entity,
            Request {
                command: Command::Gev,
            },
        );

        let network_data = server.world.get_component::<NetworkData>(entity).unwrap();
        assert_eq!(network_data.pending_responses.len(), 1);
        assert_eq!(
            network_data.pending_responses[0].data.as_ref().unwrap(),
            "gev gravity_well 3 7"
        );
    }
}
