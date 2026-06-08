use log::error;

use crate::ecs::components::network::NetworkData;
use crate::ecs::components::task::{TASK_NOT_STARTED, Task, TaskList, TaskType};
use crate::ecs::components::team::Team;
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
        let network_data = server.world.get_component_mut::<NetworkData>(entity);
        if network_data.is_none() {
            return;
        }
        let network_data = network_data.unwrap();

        team_name = match request.command {
            Command::Unknown(team_name) => team_name,
            _ => {
                network_data
                    .pending_responses
                    .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
                return;
            }
        };

        if team_name == "GRAPHIC" {
            let team = server.world.get_component_mut::<Team>(entity).unwrap();
            *team = Team::AuthenticatedGUI;
            return;
        }

        if !server.team_names.contains(&team_name) {
            network_data
                .pending_responses
                .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
            return;
        }
    }

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
    let team_component = server.world.get_component_mut::<Team>(entity).unwrap();
    *team_component = Team::AuthenticatedAI(team_name.clone());

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
}

#[cfg(test)]
mod tests {
    use crate::ecs::{builders::inhabitants::build_inhabitant, components::network};

    use super::*;

    #[test]
    fn test_inexisting_team() {
        let mut server = Server {
            listener: std::net::TcpListener::bind("127.0.0.1:0").unwrap(),
            _users: std::collections::HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: crate::ecs::storage::World::new(
                crate::ecs::map_size::MapSize {
                    width: 10,
                    height: 10,
                },
                100,
            ),
            clients_nb: 1,
            team_names: vec!["existing_team".to_string()],
        };

        let (mock_socket, output) = network::MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        let inhabitant = build_inhabitant(
            0,
            0,
            crate::utils::orientation::RelativeOrientation::Forward,
            &mut server.world,
            network_data,
        );

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
            listener: std::net::TcpListener::bind("127.0.0.1:0").unwrap(),
            _users: std::collections::HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: crate::ecs::storage::World::default(),
            clients_nb: 1,
            team_names: vec!["existing_team".to_string()],
        };

        let (mock_socket, _) = network::MockSocket::new(vec![]);
        let network_data = NetworkData::new(mock_socket);
        let inhabitant = build_inhabitant(
            0,
            0,
            crate::utils::orientation::RelativeOrientation::Forward,
            &mut server.world,
            network_data,
        );

        let request = Request {
            command: Command::Unknown("GRAPHIC".to_string()),
        };

        handle_auth_request(&mut server, inhabitant, request);

        let team = server.world.get_component::<Team>(inhabitant).unwrap();
        assert_eq!(*team, Team::AuthenticatedGUI);
    }
}
