pub mod client;
pub mod commands;
pub mod network;
pub mod signal;

use crate::ecs::components::task::TaskType;
use crate::ecs::storage::World;
use crate::ecs::systems::task::any_finished_task;
use crate::game::*;
use crate::protocol::{Request, Response, ServerEvent};
use crate::server::client::{Client, ClientState};
use nix::poll::{PollFd, PollFlags};
use std::collections::HashMap;
use std::io::Write;
use std::net::TcpListener;
use std::os::fd::AsFd;

pub struct Server {
    pub listener: TcpListener,
    pub clients: HashMap<String, Client>,
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
            clients: HashMap::new(),
            _users: HashMap::new(),
            _teams: HashMap::new(),
            _freq: 100,
            game_start: Date::now().to_timestamp(),
            world: World::new(),
        }
    }

    pub fn run(&mut self) {
        let mut fds = vec![PollFd::new(
            self.listener.as_fd(),
            PollFlags::POLLIN | PollFlags::POLLOUT,
        )];
        let client_keys: Vec<String> = self.clients.keys().cloned().collect();

        for uuid in &client_keys {
            if let Some(client) = self.clients.get(uuid) {
                fds.push(PollFd::new(
                    client.socket.as_fd(),
                    PollFlags::POLLIN | PollFlags::POLLOUT,
                ));
            }
        }

        if let Err(_e) = nix::poll::poll(&mut fds, None::<u16>) {
            return;
        }

        let listener_ready = fds[0]
            .revents()
            .is_some_and(|f| f.contains(PollFlags::POLLIN));

        let client_revents: Vec<PollFlags> = fds[1..]
            .iter()
            .map(|fd| fd.revents().unwrap_or(PollFlags::empty()))
            .collect();

        drop(fds);

        if listener_ready {
            self.accept_connections();
        }

        self.process_client_events(client_revents, client_keys);

        let finished_tasks = any_finished_task(&mut self.world, self._freq);
        for (uuid, response) in finished_tasks {
            if let Some(client) = self.clients.get_mut(&uuid) {
                client.pending_responses.push(response);
            }
        }
    }

    pub fn accept_connections(&mut self) {
        network::accept_connections(self);
    }

    pub fn process_client_events(
        &mut self,
        client_revents: Vec<PollFlags>,
        client_keys: Vec<String>,
    ) {
        network::process_client_events(self, client_revents, client_keys);
    }

    pub fn queue_task(&mut self, client_uuid: &str, task_type: TaskType) {
        commands::queue_task(self, client_uuid, task_type);
    }

    pub fn handle_request(&mut self, client_uuid: &str, request: Request) {
        commands::handle_request(self, client_uuid, request);
    }

    pub fn handle_response(&mut self, client_uuid: &str, response: Response) {
        let client = self.clients.get_mut(client_uuid).unwrap();
        let _ = client.socket.write_all(response.to_string().as_ref());
    }

    pub fn broadcast_global(&mut self, event: ServerEvent) {
        for client in self.clients.values_mut() {
            let formatted = match &client.state {
                ClientState::AuthenticatedGUI => event.to_gui_string(),
                ClientState::AuthenticatedAI(_) => None,
                ClientState::WaitingForTeamName => None,
            };

            if let Some(data) = formatted {
                client.pending_responses.push(Response::new(
                    crate::protocol::ResponseCode::Status(crate::protocol::StatusCode::Ok),
                    Some(data),
                ));
            }
        }
    }

    pub fn save(&mut self) {}
    pub fn load(&mut self) {
        let width = self.world.mapSize.width;
        let height = self.world.mapSize.height;
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
        use crate::ecs::components::inventory::Inventory;
        use crate::ecs::components::level::Level;
        use crate::ecs::components::position::Position;

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let port = listener.local_addr().unwrap().port();

        let mut server = Server {
            listener,
            clients: HashMap::new(),
            _users: HashMap::new(),
            _teams: HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: World::new(),
        };
        server.world.register_component::<TaskList>();
        server.world.register_component::<Position>();
        server.world.register_component::<Inventory>();
        server.world.register_component::<RelativeOrientation>();
        server.world.register_component::<Level>();

        let client_socket = std::net::TcpStream::connect(format!("127.0.0.1:{}", port)).unwrap();
        let mut client = Client::new(client_socket);
        let entity = build_inhabitant(0, 0, RelativeOrientation::Forward, &mut server.world);
        client.entity = Some(entity);
        let uuid = client.uuid.clone();
        server.clients.insert(uuid.clone(), client);

        for _ in 0..15 {
            server.queue_task(&uuid, TaskType::Forward);
        }

        let task_list = server.world.get_component::<TaskList>(entity).unwrap();
        assert_eq!(task_list.vector.len(), 10);
    }
}
