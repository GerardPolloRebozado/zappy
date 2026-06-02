pub mod client;
pub mod signal;

use crate::ecs::builders::inhabitants::build_inhabitant;
use crate::ecs::storage::World;
use crate::game::*;
use crate::protocol::{Command, Request, Response, ResponseCode, ServerEvent, StatusCode};
use crate::server::client::{Client, ClientState};
use nix::poll::{PollFd, PollFlags};
use std::collections::HashMap;
use std::io::Write;
use std::net::TcpListener;
use std::os::fd::AsFd;

pub struct Map {
    pub width: u32,
    pub height: u32,
}

pub struct Server {
    pub listener: TcpListener,
    pub clients: HashMap<String, Client>,
    pub _users: HashMap<String, User>,
    pub _teams: HashMap<String, Team>,
    pub map: Map,
    pub _freq: u32,
    pub game_start: u64,
    pub world: World,
    // TODO: Add a task scheduler for time management (action / f delay)
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
            map: Map {
                width: 100,
                height: 100,
            },
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
    }

    pub fn accept_connections(&mut self) {
        if let Ok((mut socket, _addr)) = self.listener.accept() {
            let _ = socket.write_all(b"WELCOME\n");
            let new_client = Client::new(socket);
            self.clients.insert(new_client.uuid.clone(), new_client);
        }
    }

    pub fn process_client_events(
        &mut self,
        client_revents: Vec<PollFlags>,
        client_keys: Vec<String>,
    ) {
        let mut disconnected = Vec::new();
        let mut requests_to_read = Vec::new();
        let mut requests_to_write = Vec::new();

        for (i, uuid) in client_keys.into_iter().enumerate() {
            let revents = client_revents[i];

            if revents.contains(PollFlags::POLLIN)
                && let Some(client) = self.clients.get_mut(&uuid)
            {
                match client.read_data() {
                    Some(msg) => {
                        requests_to_read.push((uuid.clone(), msg));
                    }
                    None => disconnected.push(uuid.clone()),
                }
            }
            if revents.contains(PollFlags::POLLOUT) {
                requests_to_write.push(uuid.clone())
            }
        }

        for (uuid, msg) in requests_to_read {
            for line in msg.split('\n') {
                let trimmed = line.trim();
                if trimmed.is_empty() {
                    continue;
                }
                match trimmed.parse::<Request>() {
                    Ok(req) => {
                        self.handle_request(&uuid, req);
                    }
                    Err(_) => {
                        if let Some(client) = self.clients.get_mut(&uuid) {
                            client.pending_responses.push(Response {
                                code: ResponseCode::Status(StatusCode::Ko),
                                data: None,
                            });
                        }
                    }
                }
            }
        }

        for uuid in requests_to_write {
            let client = self.clients.get_mut(&uuid);
            if let Some(c) = client {
                let responses: Vec<Response> = c.pending_responses.drain(..).collect();
                for response in responses {
                    self.handle_response(uuid.as_str(), response);
                }
            }
        }

        for uuid in disconnected {
            if let Some(_client) = self.clients.remove(&uuid) {
                // TODO: decide what to do when a user disconnects
            }
        }
    }

    pub fn handle_request(&mut self, client_uuid: &str, request: Request) {
        let client_state = {
            let client = self.clients.get(client_uuid).unwrap();
            client.state.clone()
        };

        if client_state == ClientState::WaitingForTeamName {
            match request.command {
                Command::Unknown(team_name) => {
                    if team_name == "GRAPHIC" {
                        if let Some(client) = self.clients.get_mut(client_uuid) {
                            client.state = ClientState::AuthenticatedGUI;
                        }
                    } else if let Some(client) = self.clients.get_mut(client_uuid) {
                        client.state = ClientState::AuthenticatedAI(team_name);

                        client.pending_responses.push(Response::new(
                            ResponseCode::Status(StatusCode::Ok),
                            Some("1".to_string()),
                        ));

                        build_inhabitant(0, 0, 1, &mut self.world);

                        client.pending_responses.push(Response::new(
                            ResponseCode::Status(StatusCode::Ok),
                            Some(format!("{} {}", self.map.width, self.map.height)),
                        ));
                    }
                }

                _ => {
                    if let Some(client) = self.clients.get_mut(client_uuid) {
                        client
                            .pending_responses
                            .push(Response::new(ResponseCode::Status(StatusCode::Ko), None));
                    }
                }
            }

            return;
        }

        match request.command {
            Command::Forward => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client
                        .pending_responses
                        .push(Response::new(ResponseCode::Status(StatusCode::Ok), None));
                }
            }

            // TODO: Implement Look command logic
            Command::Look => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client.pending_responses.push(Response::new(
                        ResponseCode::Status(StatusCode::Ok),
                        Some("[player, food, ...]".to_string()),
                    ));
                }
            }

            // TODO: Implement Inventory command logic
            Command::Inventory => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client.pending_responses.push(Response::new(
                        ResponseCode::Status(StatusCode::Ok),
                        Some("[food 10, linemate 0, ...]".to_string()),
                    ));
                }
            }

            // TODO: Implement Incantation command logic
            Command::Incantation => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client.pending_responses.push(Response::new(
                        ResponseCode::Status(StatusCode::Ok),
                        Some("Elevation underway".to_string()),
                    ));
                }
            }

            Command::Msz => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client.pending_responses.push(Response::new(
                        ResponseCode::Status(StatusCode::Ok),
                        Some(format!("msz {} {}", self.map.width, self.map.height)),
                    ));
                }
            }

            Command::Bct(x, y) => {
                if let Some(data) = self.get_tile_content(x, y)
                    && let Some(client) = self.clients.get_mut(client_uuid)
                {
                    client.pending_responses.push(Response::new(
                        ResponseCode::Status(StatusCode::Ok),
                        Some(data),
                    ));
                }
            }

            Command::Mct => {
                println!("Protocol: Received mct from {}", client_uuid);
                let mut responses = Vec::new();
                for y in 0..self.map.height {
                    for x in 0..self.map.width {
                        if let Some(data) = self.get_tile_content(x, y) {
                            responses.push(data);
                        }
                    }
                }
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    for r in responses {
                        client
                            .pending_responses
                            .push(Response::new(ResponseCode::Status(StatusCode::Ok), Some(r)));
                    }
                }
            }

            Command::Unknown(_) => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client.pending_responses.push(Response {
                        code: ResponseCode::Status(StatusCode::Ko),
                        data: None,
                    });
                }
            }

            _ => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client
                        .pending_responses
                        .push(Response::new(ResponseCode::Status(StatusCode::Ok), None));
                }
            }
        }
    }
    pub fn handle_response(&mut self, client_uuid: &str, response: Response) {
        let client = self.clients.get_mut(client_uuid).unwrap();
        let _ = client.socket.write_all(response.to_string().as_ref());
    }

    //TODO event.to_ai_string
    //It is missing bc we have to decide what to do with the users and players
    pub fn broadcast_global(&mut self, event: ServerEvent) {
        for client in self.clients.values_mut() {
            let formatted = match &client.state {
                ClientState::AuthenticatedGUI => event.to_gui_string(),
                ClientState::AuthenticatedAI(_) => None,
                ClientState::WaitingForTeamName => None,
            };

            if let Some(data) = formatted {
                client.pending_responses.push(Response::new(
                    ResponseCode::Status(StatusCode::Ok),
                    Some(data),
                ));
            }
        }
    }

    pub fn save(&mut self) {}
    pub fn load(&mut self) {
        crate::ecs::systems::tile_system::setup_map(
            &mut self.world,
            self.map.width,
            self.map.height,
        );
    }

    fn get_tile_content(&self, x: u32, y: u32) -> Option<String> {
        use crate::ecs::components::inventory::Inventory;
        use crate::ecs::components::position::Position;
        use crate::ecs::components::terrain_type::TerrainType;
        use crate::ecs::components::tile::Tile;

        let tiles = self.world.get_storage::<Tile>()?;
        let positions = self.world.get_storage::<Position>()?;
        let inventories = self.world.get_storage::<Inventory>()?;
        let terrains = self.world.get_storage::<TerrainType>()?;

        for (ent, _tile) in tiles.iter() {
            if let Some(pos) = positions.get(*ent)
                && pos.x == x
                && pos.y == y
            {
                let inv = inventories.get(*ent)?;
                let terrain = terrains.get(*ent)?;

                let food = inv.items.get(&Resource::Food).unwrap_or(&0);
                let linemate = inv.items.get(&Resource::Linemate).unwrap_or(&0);
                let deraumere = inv.items.get(&Resource::Deraumere).unwrap_or(&0);
                let sibur = inv.items.get(&Resource::Sibur).unwrap_or(&0);
                let mendiane = inv.items.get(&Resource::Mendiane).unwrap_or(&0);
                let phiras = inv.items.get(&Resource::Phiras).unwrap_or(&0);
                let thystame = inv.items.get(&Resource::Thystame).unwrap_or(&0);

                let t_type = match terrain {
                    TerrainType::Grass => 0,
                    TerrainType::Mountain => 1,
                    TerrainType::Water => 2,
                    TerrainType::Sand => 3,
                    TerrainType::Forest => 4,
                    TerrainType::ObsidianBarrens => 5,
                    TerrainType::LuminousOrchards => 6,
                    TerrainType::CrystalCanyons => 7,
                    TerrainType::MagneticTundra => 8,
                };

                return Some(format!(
                    "bct {} {} {} {} {} {} {} {} {} {}",
                    x, y, food, linemate, deraumere, sibur, mendiane, phiras, thystame, t_type
                ));
            }
        }
        None
    }
}
