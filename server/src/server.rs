//! TCP server: poll loop, client map, and timed task completion.

use crate::commands;
use crate::ecs::components::inhabitant::Inhabitant;
use crate::ecs::components::network::NetworkData;
use crate::ecs::components::task::TaskType;
use crate::ecs::components::team::Team;
use crate::ecs::storage::{Entity, World};
use crate::ecs::systems::network::network_system;
use crate::ecs::systems::run::run_systems;
use crate::ecs::systems::tile_system::{setup_map, spawn_egg};
use crate::protocol::{Request, Response, ResponseCode, ServerEvent, StatusCode};
use crate::utils::Config;
use crate::utils::date::Date;
use log::{error, info};
use nix::poll::{PollFd, PollFlags};
use std::collections::HashMap;
use std::io::Write;
use std::net::TcpListener;
use std::os::fd::AsFd;

pub struct Server {
    pub listener: Option<TcpListener>,
    pub _users: HashMap<String, Inhabitant>,
    pub _freq: u32,
    pub game_start: u64,
    pub world: World,
    pub clients_nb: u32,
    pub team_names: Vec<String>,
}

impl Default for Server {
    fn default() -> Self {
        Self::new(Config::default())
    }
}

impl Server {
    pub fn new(config: Config) -> Server {
        let listener = if config.port != 0 {
            let l = TcpListener::bind(format!("0.0.0.0:{}", config.port))
                .expect("Error starting server");
            l.set_nonblocking(true).expect("Cannot set non-blocking");
            info!("Server started on port: {}", config.port);
            Some(l)
        } else {
            None
        };
        let game_start = Date::now().to_timestamp();
        Server {
            listener,
            _users: HashMap::new(),
            _freq: config.freq,
            game_start,
            world: World::new(
                crate::ecs::map_size::MapSize {
                    width: config.width,
                    height: config.height,
                },
                config.freq as u64,
                game_start,
            ),
            clients_nb: config.clients_nb,
            team_names: config.names,
        }
    }

    /// One main-loop tick: accept connections, read client input, advance the ECS task queue.
    pub fn run(&mut self) {
        if self.listener.is_some() {
            network_system(self);
        }
        run_systems(&mut self.world);
    }

    pub fn accept_connections(&mut self) {
        let listener = match &self.listener {
            Some(l) => l,
            None => return,
        };
        while let Ok((socket, _addr)) = listener.accept() {
            let _ = socket.set_nonblocking(true);
            let mut socket = socket;
            let _ = socket.write_all(b"WELCOME\n");
            let network_data = NetworkData::new(socket);
            let entity = self.world.spawn();
            self.world
                .add_component::<NetworkData>(entity, network_data);
            self.world
                .add_component::<Team>(entity, Team::WaitingForTeamName);
            info!("New client connected: entity {}", entity.id());
        }
    }

    pub fn get_server_events(&self) -> Vec<PollFd<'_>> {
        let mut fds = Vec::new();
        if let Some(listener) = &self.listener {
            fds.push(PollFd::new(
                listener.as_fd(),
                PollFlags::POLLIN | PollFlags::POLLOUT,
            ));
        }

        if let Some(network_data_keys) = self.world.get_storage::<NetworkData>() {
            for (_, network_data) in network_data_keys.iter() {
                if let Some(socket) = &network_data.socket {
                    fds.push(PollFd::new(
                        socket.as_fd(),
                        PollFlags::POLLIN | PollFlags::POLLOUT,
                    ));
                }
            }
        }

        if fds.is_empty() {
            return fds;
        }

        if let Err(_e) = nix::poll::poll(&mut fds, 1u16) {
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

            let mut current_fd_idx = 0;
            for (entity, network_data) in network_data_storage.iter_mut() {
                if network_data.socket.is_none() {
                    continue;
                }
                let revents = client_revents[current_fd_idx];
                current_fd_idx += 1;

                if revents.contains(PollFlags::POLLOUT) {
                    requests_to_write.push(*entity);
                }

                if revents.contains(PollFlags::POLLHUP) {
                    disconnected.push(*entity);
                }

                if !revents.contains(PollFlags::POLLIN) {
                    continue;
                }

                if let Some(msg) = network_data.read_data() {
                    requests_to_read.push((*entity, msg))
                }
            }
        }

        for (entity, msg) in requests_to_read {
            for line in msg.split('\n') {
                let trimmed = line.trim();
                if trimmed.is_empty() {
                    continue;
                }

                info!("Parsing request: {}", trimmed);
                let req = match trimmed.parse::<Request>() {
                    Ok(req) => req,
                    Err(_) => {
                        error!("Cannot parse request: {}", trimmed);
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
                let limit = std::cmp::min(c.pending_responses.len(), 100);
                responses = c.pending_responses.drain(..limit).collect();
            }
            for response in responses {
                self.handle_response(entity, response);
            }
        }

        for entity in disconnected {
            self.world.despawn(entity);
        }
    }

    pub fn queue_task(&mut self, entity: Entity, task_type: TaskType) {
        commands::queue_task(self, entity, task_type);
    }

    pub fn handle_request(&mut self, entity: Entity, request: Request) {
        commands::handle_request(self, entity, request);
    }

    pub fn handle_response(&mut self, entity: Entity, response: Response) {
        info!(
            "Handling response for entity {}: {:?}",
            entity.id(),
            response
        );
        let network_data = self.world.get_component_mut::<NetworkData>(entity);
        if network_data.is_none() {
            return;
        }
        let network_data = network_data.unwrap();
        if let Some(socket) = &mut network_data.socket {
            let _ = socket.write_all(response.to_string().as_ref());
        }
    }

    pub fn broadcast_global(&mut self, event: ServerEvent) {
        crate::ecs::systems::task::broadcast_event(&mut self.world, event);
    }

    pub fn save(&mut self) {}
    pub fn load(&mut self) {
        let width = self.world.map_size.width;
        let height = self.world.map_size.height;
        setup_map(&mut self.world, width, height);
        for team in self.team_names.clone() {
            for _ in 0..self.clients_nb {
                spawn_egg(self.world.map_size, &mut self.world, 0, team.clone());
            }
        }
    }

    pub fn get_tile_content(&self, x: u32, y: u32) -> Option<String> {
        crate::ecs::map_size::get_tile_content(&self.world, x, y)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::builders::inhabitants::build_inhabitant_with_entity;
    use crate::ecs::components::network::NetworkData;
    use crate::ecs::components::task::{TaskList, TaskType};
    use crate::ecs::components::team::Team;
    use crate::utils::orientation::RelativeOrientation;
    use std::net::TcpListener;

    #[test]
    fn test_queue_task_limit() {
        let mut server = Server::default();
        let client_socket = std::net::TcpStream::connect(format!("127.0.0.1:{}", 8080)).unwrap();
        let network = NetworkData::new(client_socket);
        let entity = server.world.spawn();
        let entity = build_inhabitant_with_entity(
            entity,
            0,
            0,
            RelativeOrientation::Forward,
            &mut server.world,
        );
        server.world.add_component(entity, network);

        for _ in 0..15 {
            server.queue_task(entity, TaskType::Forward);
        }

        let task_list = server.world.get_component::<TaskList>(entity).unwrap();
        assert_eq!(task_list.vector.len(), 10);
    }

    #[test]
    fn test_broadcast_global_ai_per_receiver_k() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap();

        let mut server = Server {
            listener: Some(listener),
            _users: HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: World::new(
                crate::ecs::map_size::MapSize {
                    width: 10,
                    height: 10,
                },
                100,
                0,
            ),
            clients_nb: 1,
            team_names: vec!["team".to_string()],
        };

        let socket_a = std::net::TcpStream::connect(addr).unwrap();
        let network_a = NetworkData::new(socket_a);
        let entity_same = server.world.spawn();
        let entity = build_inhabitant_with_entity(
            entity_same,
            5,
            5,
            RelativeOrientation::Forward,
            &mut server.world,
        );
        server.world.add_component(entity, network_a);
        if let Some(nd) = server.world.get_component_mut::<Team>(entity_same) {
            *nd = Team::AuthenticatedAI("team".to_string());
        }

        let socket_b = std::net::TcpStream::connect(addr).unwrap();
        let network_b = NetworkData::new(socket_b);
        let entity_east = server.world.spawn();
        let entity_east = build_inhabitant_with_entity(
            entity_east,
            6,
            5,
            RelativeOrientation::Forward,
            &mut server.world,
        );
        server.world.add_component(entity_east, network_b);
        if let Some(nd) = server.world.get_component_mut::<Team>(entity_east) {
            *nd = Team::AuthenticatedAI("team".to_string());
        }

        let event = ServerEvent::Message {
            player_id: 1,
            message: "hello".to_string(),
            x: 5,
            y: 5,
        };
        server.broadcast_global(event);

        let network_a = server
            .world
            .get_component::<NetworkData>(entity_same)
            .unwrap();
        let network_b = server
            .world
            .get_component::<NetworkData>(entity_east)
            .unwrap();

        let line_a = network_a.pending_responses[0].data.as_ref().unwrap();
        let line_b = network_b.pending_responses[0].data.as_ref().unwrap();

        assert_eq!(line_a, "message 0, hello");
        assert_eq!(line_b, "message 3, hello");
    }
}
