use crate::commands::{handle_login, user::{handle_user, handle_users}};
use zappy::common::StatusCode;
use zappy::common::client::Client;
use zappy::common::protocol::ResponseCode;
use zappy::common::protocol::command::Command;
use zappy::common::protocol::request::Request;
use zappy::common::user::User;
use zappy::common::utils::escape_str;
use zappy::common::{Response, Team};
use nix::poll::{PollFd, PollFlags};
use std::collections::HashMap;
use std::io::Write;
use std::net::TcpListener;
use std::os::fd::AsFd;

pub struct Server {
    pub listener: TcpListener,
    pub clients: HashMap<String, Client>,
    pub users: HashMap<String, User>,
    pub teams: HashMap<String, Team>,
}

impl Server {
    pub fn new() -> Server {
        Server {
            listener: TcpListener::bind("0.0.0.0:8080")
                .expect("Error starting server on port 8080"),
            clients: HashMap::new(),
            users: HashMap::new(),
            teams: HashMap::new(),
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
            .map_or(false, |f| f.contains(PollFlags::POLLIN));

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
        if let Ok((socket, _addr)) = self.listener.accept() {
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

            if revents.contains(PollFlags::POLLIN) {
                if let Some(client) = self.clients.get_mut(&uuid) {
                    match client.read_data() {
                        Some(msg) => {
                            let trimmed = msg.trim();
                            if trimmed == "/disconnect" {
                                disconnected.push(uuid);
                                continue;
                            } else if trimmed == "LOGOUT" {
                                disconnected.push(uuid.clone());
                            } else {
                                match trimmed.parse::<Request>() {
                                    Ok(req) => {
                                        if client.user.is_none()
                                            && !matches!(req.command, Command::Login(_))
                                        {
                                            client.pending_responses.push(Response {
                                                code: ResponseCode::Status(
                                                    StatusCode::Unauthorized,
                                                ),
                                                data: None,
                                            });
                                            requests_to_write.push(uuid.clone());
                                            continue;
                                        }
                                        requests_to_read.push((uuid.clone(), req));
                                    }
                                    Err(_) => {
                                        if let Some(client) = self.clients.get_mut(&uuid) {
                                            client.pending_responses.push(Response {
                                                code: ResponseCode::Status(StatusCode::BadRequest),
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
            match client {
                None => {}
                Some(c) => {
                    let responses: Vec<Response> = c.pending_responses.drain(..).collect();
                    for response in responses {
                        self.handle_response(uuid.as_str(), response);
                    }
                }
            }
        }

        for uuid in disconnected {
            if let Some(client) = self.clients.remove(&uuid) {
                if let Some(user_uuid) = &client.user {
                    if let Some(user) = self.users.get(user_uuid) {
                        let event_resp = Response {
                            code: ResponseCode::Event(
                                zappy::common::protocol::event::EventCode::LoggedOut,
                            ),
                            data: Some(format!("\"{}\" \"{}\"", user.uuid, escape_str(&user.name))),
                        };
                        self.broadcast_global(event_resp);
                    }
                }
            }
        }
    }

    pub fn handle_request(&mut self, client_uuid: &str, request: Request) {
        match request.command {
            Command::Login(name) => handle_login(self, client_uuid, name),
            Command::Users => handle_users(self, client_uuid),
            Command::User(uuid) => handle_user(self, client_uuid, &uuid),
            Command::Unknown(cmd) => {
                println!("Unknown command received: {}", cmd);
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client.pending_responses.push(Response {
                        code: ResponseCode::Status(StatusCode::BadRequest),
                        data: None,
                    });
                }
            }
            _ => {
                if let Some(client) = self.clients.get_mut(client_uuid) {
                    client.pending_responses.push(Response {
                        code: ResponseCode::Status(StatusCode::BadRequest),
                        data: None,
                    });
                }
            }
        }
    }

    pub fn handle_response(&mut self, client_uuid: &str, response: Response) {
        let client = self.clients.get_mut(client_uuid).unwrap();
        let _ = client.socket.write_all(response.to_string().as_ref());
    }

    pub fn broadcast_global(&mut self, event: Response) {
        for client in self.clients.values_mut() {
            if client.user.is_some() {
                client.pending_responses.push(event.clone());
            }
        }
    }

    pub fn broadcast_to_team(&mut self, team_members: &[String], event: Response) {
        for client in self.clients.values_mut() {
            if let Some(user_uuid) = &client.user {
                if team_members.contains(user_uuid) {
                    client.pending_responses.push(event.clone());
                }
            }
        }
    }

    pub fn broadcast_to_team_except(
        &mut self,
        team_members: &[String],
        exclude_uuid: &str,
        event: Response,
    ) {
        for client in self.clients.values_mut() {
            if let Some(user_uuid) = &client.user {
                if user_uuid != exclude_uuid && team_members.contains(user_uuid) {
                    client.pending_responses.push(event.clone());
                }
            }
        }
    }

    pub fn send_to_user(&mut self, target_uuid: &str, event: Response) {
        for client in self.clients.values_mut() {
            if let Some(user_uuid) = &client.user {
                if user_uuid == target_uuid {
                    client.pending_responses.push(event.clone());
                }
            }
        }
    }

    pub fn save(&mut self) {
        for client in self.clients.values_mut() {
            let _ = client.socket.shutdown(std::net::Shutdown::Both);
        }
        self.clients.clear();
    }

    pub fn load(&mut self) {
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use zappy::common::protocol::status::StatusCode;
    use std::net::TcpListener;

    fn create_test_server() -> Server {
        Server {
            listener: TcpListener::bind("127.0.0.1:0").unwrap(),
            clients: HashMap::new(),
            users: HashMap::new(),
            teams: HashMap::new(),
        }
    }

    #[test]
    fn test_server_broadcast_global() {
        let mut server = create_test_server();
        let event = Response {
            code: ResponseCode::Status(StatusCode::Ok),
            data: Some("test".to_string()),
        };
        server.broadcast_global(event);
    }

    #[test]
    fn test_server_accept_connections() {
        use std::net::TcpStream;
        let mut server = create_test_server();
        let addr = server.listener.local_addr().unwrap();

        let _client_socket = TcpStream::connect(addr).unwrap();
        server.accept_connections();

        assert_eq!(server.clients.len(), 1);
    }

    #[test]
    fn test_server_handle_request_login() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            zappy::common::client::Client::new(socket),
        );

        let req = Request {
            command: Command::Login("testuser".to_string()),
        };
        server.handle_request(client_uuid, req);

        assert!(server.users.values().any(|u| u.name == "testuser"));
    }
}
