//! TCP server: poll loop, client map, and timed task completion.

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
        Server {
            listener: TcpListener::bind(format!("0.0.0.0:{}", config.port))
                .expect("Error starting server"),
            clients: HashMap::new(),
            _users: HashMap::new(),
            _teams: HashMap::new(),
            map: Map {
                width: config.width,
                height: config.height,
            },
            _freq: config.freq,
            game_start: Date::now().to_timestamp(),
            world: World::new(),
            clients_nb: config.clients_nb,
            team_names: config.names
        }
    }

    /// One main-loop tick: accept connections, read client input, advance the ECS task queue.
    ///
    /// Finished tasks from [`any_finished_task`] yield command replies and optional
    /// [`ServerEvent`]s. Replies are queued first on the acting client's
    /// [`Client::pending_responses`], then each event is fan-out with
    /// [`Self::broadcast_global`].
    ///
    /// For `Broadcast <text>` (see [`TaskType::BroadcastText`]):
    ///
    /// ```text
    /// AI  -->  "Broadcast hello\n"     queue task, 7/f timer
    /// AI  <--  "ok\n"                   command reply
    /// AI  <--  "message k, hello\n"     all AIs (k per receiver)
    /// GUI <--  "pbc #n hello\n"
    /// ```
    ///
    /// The broadcaster gets both `ok` and their own `message` line; `k` comes from
    /// [`ServerEvent::to_ai_string`].
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

        let (finished_tasks, events) = any_finished_task(&mut self.world, self._freq);
        // Command replies first (e.g. `ok` for a finished Broadcast).
        for (uuid, response) in finished_tasks {
            if let Some(client) = self.clients.get_mut(&uuid) {
                client.pending_responses.push(response);
            }
        }
        // Then world notifications (`message` / `pbc`, …).
        for event in events {
            self.broadcast_global(event);
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

    /// Returns the map snapshot for an AI client ready to receive broadcast lines.
    ///
    /// `None` when the client is not authenticated AI, has no [`Client::entity`], or
    /// the entity is missing position/orientation in [`Self::world`].
    fn ai_client_ok(client: &Client, world: &World) -> Option<Inhabitant> {
        let ClientState::AuthenticatedAI(_) = &client.state else {
            return None;
        };
        let entity = client.entity?;
        Inhabitant::get(entity, world)
    }

    /// Pushes a [`ServerEvent`] to every connected client that should see it.
    ///
    /// - **GUI** ([`ClientState::AuthenticatedGUI`]): [`ServerEvent::to_gui_string`]
    ///   (e.g. `pbc #3 hello\n` for [`ServerEvent::Message`]).
    /// - **AI** ([`ClientState::AuthenticatedAI`]): [`ServerEvent::to_ai_string`] with
    ///   that client’s [`Inhabitant`] snapshot so `k` is relative to the receiver
    ///   (e.g. `message 2, hello\n`). Skips AI clients with no linked `entity`.
    ///
    /// Uses a two-pass collect-then-push pattern so [`Self::world`] and [`Self::clients`]
    /// are not borrowed mutably at the same time. Lines are queued on
    /// [`Client::pending_responses`] like ordinary protocol output.
    pub fn broadcast_global(&mut self, event: ServerEvent) {
        let mut ai_lines: Vec<(String, String)> = Vec::new();
        
        for (uuid, client) in &self.clients {
            if let Some(inhabitant) = Self::ai_client_ok(client, &self.world) {
                if let Some(line) = event.to_ai_string(
                    Some(&inhabitant),
                    self.world.map_size.width,
                    self.world.map_size.height,
                ) {
                    ai_lines.push((uuid.clone(), line));
                }
            }
        }

        for client in self.clients.values_mut() {
            if let ClientState::AuthenticatedGUI = &client.state {
                if let Some(data) = event.to_gui_string() {
                    client.pending_responses.push(Response::new(
                        crate::protocol::ResponseCode::Status(crate::protocol::StatusCode::Ok),
                        Some(data),
                    ));
                }
            }
        }

        for (uuid, line) in ai_lines {
            if let Some(client) = self.clients.get_mut(&uuid) {
                client.pending_responses.push(Response::new(
                    crate::protocol::ResponseCode::Status(crate::protocol::StatusCode::Ok),
                    Some(line),
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
            clients: HashMap::new(),
            _users: HashMap::new(),
            _teams: HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: World::new(),
        };

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

    /// Each AI receiver gets `message k, text` with `k` from their own tile/orientation.
    #[test]
    fn test_broadcast_global_ai_per_receiver_k() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap();

        let mut server = Server {
            listener,
            clients: HashMap::new(),
            _users: HashMap::new(),
            _teams: HashMap::new(),
            _freq: 100,
            game_start: 0,
            world: World::new(),
        };
        server.world.map_size.width = 10;
        server.world.map_size.height = 10;

        let entity_same = build_inhabitant(5, 5, RelativeOrientation::Forward, &mut server.world);
        let entity_east = build_inhabitant(6, 5, RelativeOrientation::Forward, &mut server.world);

        let socket_a = std::net::TcpStream::connect(addr).unwrap();
        let mut client_a = Client::new(socket_a);
        client_a.state = ClientState::AuthenticatedAI("team".to_string());
        client_a.entity = Some(entity_same);
        let uuid_a = client_a.uuid.clone();

        let socket_b = std::net::TcpStream::connect(addr).unwrap();
        let mut client_b = Client::new(socket_b);
        client_b.state = ClientState::AuthenticatedAI("team".to_string());
        client_b.entity = Some(entity_east);
        let uuid_b = client_b.uuid.clone();

        server.clients.insert(uuid_a.clone(), client_a);
        server.clients.insert(uuid_b.clone(), client_b);

        let event = ServerEvent::Message {
            player_id: 1,
            message: "hello".to_string(),
            x: 5,
            y: 5,
        };
        server.broadcast_global(event);

        let line_a = server.clients[&uuid_a].pending_responses[0]
            .data
            .as_ref()
            .unwrap();
        let line_b = server.clients[&uuid_b].pending_responses[0]
            .data
            .as_ref()
            .unwrap();

        assert_eq!(line_a, "message 0, hello\n");
        assert_eq!(line_b, "message 3, hello\n");
    }
}
