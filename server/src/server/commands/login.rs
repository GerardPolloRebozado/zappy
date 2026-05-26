use crate::server::Server;
use zappy::common::protocol::response::{Response, ResponseCode};
use zappy::common::protocol::status::StatusCode;
use zappy::common::user::User;
use zappy::common::utils::constants::MAX_NAME_LENGTH;
use zappy::common::utils::escape_str;

pub fn handle_login(server: &mut Server, client_uuid: &str, name: String) {
    if name.len() > MAX_NAME_LENGTH {
        let response = Response {
            code: ResponseCode::Status(StatusCode::BadRequest),
            data: None,
        };
        if let Some(client) = server.clients.get_mut(client_uuid) {
            client.pending_responses.push(response);
        }
        return;
    }

    let user_uuid = resolve_user(server, &name);

    perform_login(server, client_uuid, user_uuid.clone());

    send_success_response(server, client_uuid, &user_uuid, &name);
}

fn resolve_user(server: &mut Server, name: &String) -> String {
    if let Some(user) = server.users.values().find(|u| &u.name == name) {
        user.uuid.clone()
    } else {
        create_new_user(server, name)
    }
}

fn create_new_user(server: &mut Server, name: &String) -> String {
    let user = User::new(name.clone());
    let uuid = user.uuid.clone();
    server.users.insert(uuid.clone(), user);
    uuid.clone()
}

fn perform_login(server: &mut Server, client_uuid: &str, user_uuid: String) {
    if let Some(client) = server.clients.get_mut(client_uuid) {
        client.user = Some(user_uuid.clone());
    }
}

fn send_success_response(
    server: &mut Server,
    client_uuid: &str,
    user_uuid: &String,
    name: &String,
) {
    let response = Response {
        code: ResponseCode::Status(StatusCode::Ok),
        data: Some(format!("\"{}\" \"{}\"", user_uuid, escape_str(name))),
    };
    if let Some(client) = server.clients.get_mut(client_uuid) {
        client.pending_responses.push(response);
    }

    let event_resp = Response {
        code: ResponseCode::Event(zappy::common::protocol::event::EventCode::LoggedIn),
        data: Some(format!("\"{}\" \"{}\"", user_uuid, escape_str(name))),
    };
    server.broadcast_global(event_resp);
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;
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
    fn test_handle_login_too_long_name() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        // Add a mock client
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            zappy::common::client::Client::new(socket),
        );

        let long_name = "a".repeat(MAX_NAME_LENGTH + 1);
        handle_login(&mut server, client_uuid, long_name);

        // Should have a bad request response
        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            ResponseCode::Status(StatusCode::BadRequest)
        );
    }
}
