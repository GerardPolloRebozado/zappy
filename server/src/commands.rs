use crate::commands::ai::handle_ai_command;
use crate::commands::gui::handle_gui_command;
use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
use crate::ecs::components::egg::Egg;
use crate::ecs::components::network::NetworkData;
use crate::ecs::components::position::Position;
use crate::ecs::components::task::{TASK_NOT_STARTED, Task, TaskList, TaskType};
use crate::ecs::components::team::Team;
use crate::ecs::storage::Entity;
use crate::ecs::systems::task::broadcast_event;
use crate::protocol::{Command, Request, Response, ResponseCode, ServerEvent, StatusCode};
use crate::server::Server;
use crate::utils::orientation::RelativeOrientation;
use log::{error, info};

pub mod ai;
pub mod gui;
pub mod gui_sync;
pub mod ppo;

pub fn queue_task(server: &mut Server, entity: Entity, task_type: TaskType) {
    let task_list = match server.world.get_component_mut::<TaskList>(entity) {
        Some(tl) => tl,
        None => return,
    };

    if task_list.vector.len() >= 10 {
        return;
    }

    info!("Adding new task {}", task_type);
    task_list.vector.push(Task {
        task_type,
        finish_on: TASK_NOT_STARTED,
    });
}

/// given a request it check if its logged in and if its a ai or gui and then call the right handler
pub fn handle_request(server: &mut Server, entity: Entity, request: Request) {
    let team = server.world.get_component::<Team>(entity);

    if team.is_none() {
        error!("Could not find team for entity {}", entity);
        return;
    }
    let team = team.unwrap().clone();

    match team {
        Team::AuthenticatedAI(_) => handle_ai_command(server, entity, request),
        Team::WaitingForTeamName => handle_auth_request(server, entity, request),
        Team::AuthenticatedGUI => handle_gui_command(server, entity, request),
    }
}

/// this function read the message, if the message is "GRAPHIC" it will log as GRAPHIC and will recibe ok
/// in the case of another string it means its a team name and will check if its valid and if there are any slots available
pub fn handle_auth_request(server: &mut Server, entity: Entity, request: Request) {
    let width = server.world.map_size.width;
    let height = server.world.map_size.height;
    let team_name;

    {
        {
            let network_data = server.world.get_component_mut::<NetworkData>(entity);
            if network_data.is_none() {
                return;
            }
            let network_data = network_data.unwrap();

            team_name = match request.command {
                Command::Unknown(team_name) => team_name,
                _ => {
                    error!("Team not found: {}", request.command);
                    network_data
                        .pending_responses
                        .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
                    return;
                }
            };
        }

        if team_name == "GRAPHIC" {
            {
                let team = server.world.get_component_mut::<Team>(entity).unwrap();
                *team = Team::AuthenticatedGUI;
                info!("Entity {} authenticated as GRAPHIC", entity);
            }
            gui_sync::sync_players_to_gui(&mut server.world, entity);
            gui_sync::sync_eggs_to_gui(&mut server.world, entity);
            return;
        }

        let network_data = server
            .world
            .get_component_mut::<NetworkData>(entity)
            .unwrap();
        if !server.team_names.contains(&team_name) {
            network_data
                .pending_responses
                .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
            return;
        }
    }

    // check if the team is full
    let team_members = Team::team_members(team_name.clone(), &server.world);

    if team_members.iter().len() >= usize::try_from(server.clients_nb).unwrap() {
        let network_data = server
            .world
            .get_component_mut::<NetworkData>(entity)
            .unwrap();
        network_data
            .pending_responses
            .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
        return;
    }
    //

    let egg = Egg::egg_from_team(&mut server.world, team_name.clone()).unwrap();
    let egg_position = server.world.get_component::<Position>(egg).unwrap().clone();
    let entity = build_inhabitant_with_entity(
        entity,
        egg_position.x,
        egg_position.y,
        RelativeOrientation::Forward,
        &mut server.world,
    );

    let team_component = server.world.get_component_mut::<Team>(entity).unwrap();
    *team_component = Team::AuthenticatedAI(team_name.clone());
    info!(
        "Entity {} authenticated as team {}, placed at ({}, {})",
        entity, team_name, egg_position.x, egg_position.y
    );
    server.broadcast_global(ServerEvent::NewPlayer {
        player_id: entity.id(),
        x: egg_position.x,
        y: egg_position.y,
        orientation: RelativeOrientation::SameTile,
        level: 0,
        team: team_name,
    });

    let network_data = server.world.get_component_mut::<NetworkData>(entity);

    if network_data.is_none() {
        return;
    }
    let network_data = network_data.unwrap();

    network_data.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some(team_members.values().len().to_string()),
    ));

    network_data.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some(format!("{} {}", width, height)),
    ));
    broadcast_event(
        &mut server.world,
        ServerEvent::EggConnect { egg_id: egg.id() },
    );
    server.world.despawn(egg);
}

#[cfg(test)]
mod tests {
    use crate::ecs::{components::network, systems::tile_system::spawn_egg};

    use super::*;

    #[test]
    fn test_inexisting_team() {
        let mut server = Server {
            listener: Some(std::net::TcpListener::bind("127.0.0.1:0").unwrap()),
            _users: std::collections::HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: crate::ecs::storage::World::new(
                crate::ecs::map_size::MapSize {
                    width: 10,
                    height: 10,
                },
                100,
                0,
            ),
            clients_nb: 1,
            team_names: vec!["existing_team".to_string()],
        };

        let (mock_socket, output) = network::MockSocket::new(vec![]);
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

        let request = Request {
            command: Command::Unknown("non_existent".to_string()),
        };

        handle_auth_request(&mut server, inhabitant, request);

        let responses: Vec<_> = {
            let nd = server
                .world
                .get_component_mut::<NetworkData>(inhabitant)
                .unwrap();
            nd.pending_responses.drain(..).collect()
        };
        for resp in responses {
            server.handle_response(inhabitant, resp);
        }

        let output_bytes = output.lock().unwrap();
        let output_str = String::from_utf8_lossy(&output_bytes);
        assert_eq!(output_str, "ko\n");
    }

    #[test]
    fn test_graphic_auth() {
        let mut server = Server {
            listener: Some(std::net::TcpListener::bind("127.0.0.1:0").unwrap()),
            _users: std::collections::HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: crate::ecs::storage::World::default(),
            clients_nb: 1,
            team_names: vec!["existing_team".to_string()],
        };

        let ai = server.world.spawn();
        build_inhabitant_with_entity(
            ai,
            3,
            7,
            crate::utils::orientation::RelativeOrientation::Forward,
            &mut server.world,
        );
        *server.world.get_component_mut::<Team>(ai).unwrap() =
            Team::AuthenticatedAI("existing_team".to_string());

        let egg = server.world.spawn();
        server.world.add_component(
            egg,
            Egg {
                team: "existing_team".to_string(),
                player_id: 0,
            },
        );
        server.world.add_component(egg, Position { x: 5, y: 9 });

        let (mock_socket, _) = network::MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        let gui = server.world.spawn();
        build_inhabitant_with_entity(
            gui,
            0,
            0,
            crate::utils::orientation::RelativeOrientation::Forward,
            &mut server.world,
        );
        server.world.add_component(gui, network_data);

        let request = Request {
            command: Command::Unknown("GRAPHIC".to_string()),
        };

        handle_auth_request(&mut server, gui, request);

        let team = server.world.get_component::<Team>(gui).unwrap();
        assert_eq!(*team, Team::AuthenticatedGUI);

        let nd = server.world.get_component::<NetworkData>(gui).unwrap();
        let response_data: Vec<&str> = nd
            .pending_responses
            .iter()
            .filter_map(|r| r.data.as_deref())
            .collect();

        assert!(
            response_data.iter().any(|d| d.starts_with("pnw")),
            "expected pnw in responses, got: {:?}",
            response_data
        );
        assert!(
            response_data.iter().any(|d| d.starts_with("enw")),
            "expected enw in responses, got: {:?}",
            response_data
        );
    }

    #[test]
    fn test_ai_auth() {
        let mut server = Server::default();

        let (mock_socket, _) = network::MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        spawn_egg(
            server.world.map_size,
            &mut server.world,
            0,
            "TeamA".to_string(),
        );
        let inhabitant = server.world.spawn();
        build_inhabitant_with_entity(
            inhabitant,
            0,
            0,
            crate::utils::orientation::RelativeOrientation::Forward,
            &mut server.world,
        );
        server.world.add_component(inhabitant, network_data);

        let request = Request {
            command: Command::Unknown("TeamA".to_string()),
        };

        handle_auth_request(&mut server, inhabitant, request);

        let team = server.world.get_component::<Team>(inhabitant).unwrap();
        assert_eq!(*team, Team::AuthenticatedAI("TeamA".to_string()));
    }

    #[test]
    fn test_ai_auth_consumes_egg() {
        let mut server = Server {
            listener: Some(std::net::TcpListener::bind("127.0.0.1:0").unwrap()),
            _users: std::collections::HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: crate::ecs::storage::World::new(
                crate::ecs::map_size::MapSize {
                    width: 10,
                    height: 10,
                },
                100,
                0,
            ),
            clients_nb: 10,
            team_names: vec!["existing_team".to_string()],
        };

        let (mock_socket, _) = network::MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        let egg = server.world.spawn();
        server.world.add_component(egg, Position { x: 5, y: 5 });
        server.world.add_component(
            egg,
            Egg {
                team: "existing_team".to_string(),
                player_id: 0,
            },
        );

        let inhabitant = server.world.spawn();
        server.world.add_component(inhabitant, network_data);
        server
            .world
            .add_component(inhabitant, Team::WaitingForTeamName);

        let request = Request {
            command: Command::Unknown("existing_team".to_string()),
        };

        handle_auth_request(&mut server, inhabitant, request);

        // Check if egg is despawned
        assert!(!server.world.is_alive(egg));
        let egg_storage = server.world.get_storage::<Egg>().unwrap();
        assert_eq!(egg_storage.iter().count(), 0);
    }
}
