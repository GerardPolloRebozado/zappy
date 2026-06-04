pub mod commands;
pub mod signal;

use crate::ecs::builders::inhabitants::build_inhabitant;
use crate::ecs::components::network::{ClientState, NetworkData};
use crate::ecs::components::task::TaskType;
use crate::ecs::storage::{Entity, World};
use crate::ecs::systems::network::network_system;
use crate::ecs::systems::run::run_systems;
use crate::game::*;
use crate::protocol::{Request, Response, ResponseCode, ServerEvent, StatusCode};
use crate::utils::orientation;
use nix::poll::{PollFd, PollFlags};
use std::collections::HashMap;
use std::io::Write;
use std::net::TcpListener;
use std::os::fd::AsFd;

pub struct Server {
    pub listener: TcpListener,
    pub _users: HashMap<String, User>,
    pub _teams: HashMap<String, Team>,
    pub _freq: u32,
    pub game_start: u64,
    pub world: World,
}

impl Default for Server {
    fn default() -> Self {
        Self::new()
    }
}

impl Server {
    pub fn new() -> Server {
        Server {
            listener: TcpListener::bind("0.0.0.0:8080")
                .expect("Error starting server on port 8080"),
            _users: HashMap::new(),
            _teams: HashMap::new(),
            _freq: 100,
            game_start: Date::now().to_timestamp(),
            world: World::default(), // TODO: change this to use the specified one, once argument parsing is done
        }
    }

    pub fn run(&mut self) {
        network_system(self);
        run_systems(&mut self.world);
    }

    pub fn accept_connections(&mut self) {
        loop {
            if let Ok((mut socket, _addr)) = self.listener.accept() {
                let _ = socket.write_all(b"WELCOME\n");
                let network_data = NetworkData::new(socket);
                build_inhabitant(
                    0,
                    0,
                    orientation::RelativeOrientation::Forward,
                    &mut self.world,
                    network_data,
                );
            } else {
                return;
            }
        }
    }

    pub fn get_server_events(&self) -> Vec<PollFd<'_>> {
        let mut fds = vec![PollFd::new(
            self.listener.as_fd(),
            PollFlags::POLLIN | PollFlags::POLLOUT,
        )];

        let network_data_keys = self.world.get_storage::<NetworkData>();
        if network_data_keys.is_none() {
            return fds;
        }
        let network_data_keys = network_data_keys.unwrap();

        for (_, network_data) in network_data_keys.iter() {
            fds.push(PollFd::new(
                network_data.socket.as_fd(),
                PollFlags::POLLIN | PollFlags::POLLOUT,
            ));
        }

        if let Err(_e) = nix::poll::poll(&mut fds, None::<u16>) {
            return fds;
        }

        fds
    }

    pub fn process_client_events(&mut self, client_revents: Vec<PollFlags>) {
        let mut disconnected = Vec::new();
        let mut requests_to_read = Vec::new();
        let mut requests_to_write = Vec::new();

        {
            let network_data_storage = self.world.get_storage_mut::<NetworkData>();
            if network_data_storage.is_none() {
                return;
            }
            let network_data_storage = network_data_storage.unwrap();

            for (i, (entity, network_data)) in network_data_storage.iter_mut().enumerate() {
                let revents = client_revents[i];

                if revents.contains(PollFlags::POLLOUT) {
                    requests_to_write.push(*entity);
                }

                if !revents.contains(PollFlags::POLLIN) {
                    continue;
                }

                match network_data.read_data() {
                    Some(msg) => requests_to_read.push((*entity, msg)),
                    None => disconnected.push(*entity),
                }
            }
        }

        for (entity, msg) in requests_to_read {
            for line in msg.split('\n') {
                let trimmed = line.trim();
                if trimmed.is_empty() {
                    continue;
                }

                let req = match trimmed.parse::<Request>() {
                    Ok(req) => req,
                    Err(_) => {
                        if let Some(client) = self.world.get_component_mut::<NetworkData>(entity) {
                            client.pending_responses.push(Response {
                                code: ResponseCode::Status(StatusCode::Ko),
                                data: None,
                            });
                        }
                        continue;
                    }
                };

                commands::handle_request(self, entity, req);
            }
        }

        for entity in requests_to_write {
            let mut responses = Vec::new();
            if let Some(c) = self.world.get_component_mut::<NetworkData>(entity) {
                responses = c.pending_responses.drain(..).collect();
            }
            for response in responses {
                self.handle_response(entity, response);
            }
        }

        // for uuid in disconnected {
        // if let Some(_client) = server.clients.remove(&uuid) {
        // TODO: decide what to do when a user disconnects
        //}
        //}
    }

    pub fn queue_task(&mut self, entity: Entity, task_type: TaskType) {
        commands::queue_task(self, entity, task_type);
    }

    pub fn handle_request(&mut self, entity: Entity, request: Request) {
        commands::handle_request(self, entity, request);
    }

    pub fn handle_response(&mut self, entity: Entity, response: Response) {
        let network_data = self.world.get_component_mut::<NetworkData>(entity);
        if network_data.is_none() {
            return;
        }
        let network_data = network_data.unwrap();
        let _ = network_data.socket.write_all(response.to_string().as_ref());
    }

    pub fn broadcast_global(&mut self, event: ServerEvent) {
        let storage = self.world.get_storage_mut::<NetworkData>();
        if storage.is_none() {
            return;
        }
        let storage = storage.unwrap();
        for (_, network_data) in storage.iter_mut() {
            let formatted = match &network_data.state {
                ClientState::AuthenticatedGUI => event.to_gui_string(),
                ClientState::AuthenticatedAI(_) => None,
                ClientState::WaitingForTeamName => None,
            };

            if let Some(data) = formatted {
                network_data.pending_responses.push(Response::new(
                    crate::protocol::ResponseCode::Status(crate::protocol::StatusCode::Ok),
                    Some(data),
                ));
            }
        }
    }

    pub fn save(&mut self) {}
    pub fn load(&mut self) {
        let width = self.world.map_size.width;
        let height = self.world.map_size.height;
        crate::ecs::systems::tile_system::setup_map(&mut self.world, width, height);
    }

    pub fn get_tile_content(&self, x: u32, y: u32) -> Option<String> {
        crate::ecs::map_size::get_tile_content(&self.world, x, y)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant;
    use crate::ecs::components::task::{TaskList, TaskType};
    use crate::utils::orientation::RelativeOrientation;
    use std::net::TcpListener;

    #[test]
    fn test_queue_task_limit() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let port = listener.local_addr().unwrap().port();

        let mut server = Server {
            listener,
            _users: HashMap::new(),
            _teams: HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: World::new(100),
        };

        let client_socket = std::net::TcpStream::connect(format!("127.0.0.1:{}", port)).unwrap();
        let network = NetworkData::new(client_socket);
        let entity = build_inhabitant(
            0,
            0,
            RelativeOrientation::Forward,
            &mut server.world,
            network,
        );

        for _ in 0..15 {
            server.queue_task(entity, TaskType::Forward);
        }

        let task_list = server.world.get_component::<TaskList>(entity).unwrap();
        assert_eq!(task_list.vector.len(), 10);
    }
}
