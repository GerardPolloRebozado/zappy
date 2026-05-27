use nix::poll::{PollFd, PollFlags};
use std::collections::HashMap;
use std::io::Write;
use std::net::TcpListener;
use std::os::fd::AsFd;
use zappy_server::common::client::{Client, ClientState};
use zappy_server::common::protocol::ResponseCode;
use zappy_server::common::*;

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
    // TODO: Add a task scheduler for time management (action / f delay)
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
                width: 10,
                height: 10,
            },
            _freq: 100,
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
                        let trimmed = msg.trim();
                        if trimmed == "/disconnect" {
                            disconnected.push(uuid);
                            continue;
                        } else {
                            match trimmed.parse::<Request>() {
                                Ok(req) => {
                                    requests_to_read.push((uuid.clone(), req));
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
                    None => disconnected.push(uuid.clone()),
                }
            }
            if revents.contains(PollFlags::POLLOUT) {
                requests_to_write.push(uuid.clone())
            }
        }

        for (uuid, req) in requests_to_read {
            self.handle_request(&uuid, req);
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
            if let Some(client) = self.clients.remove(&uuid)
                && let Some(user_uuid) = &client.user
            {
                let event_resp = Response {
                    code: ResponseCode::Event(protocol::event::EventCode::Pdi),
                    data: Some(user_uuid.clone()),
                };
                self.broadcast_global(event_resp);
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

    // TODO: Separate `broadcast_global` into a system that accepts `ServerEvent`.
    // The broadcaster should check `client.state`:
    // - If `AuthenticatedAI`: send `event.to_ai_string()`
    // - If `AuthenticatedGUI`: send `event.to_gui_string()`
    pub fn broadcast_global(&mut self, event: Response) {
        for client in self.clients.values_mut() {
            if client.user.is_some() {
                client.pending_responses.push(event.clone());
            }
        }
    }

    pub fn save(&mut self) {}
    pub fn load(&mut self) {}
}
