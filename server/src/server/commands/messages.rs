use crate::server::Server;
use myteams::common::message::Location;
use myteams::common::protocol::response::{Response, ResponseCode};
use myteams::common::protocol::status::StatusCode;

pub fn handle_messages(server: &mut Server, client_uuid: &str, other_user_uuid: String) {
    let current_user_uuid = match server.clients.get(client_uuid).and_then(|c| c.user.clone()) {
        Some(uuid) => uuid,
        None => {
            let resp = Response {
                code: ResponseCode::Status(StatusCode::Unauthorized),
                data: None,
            };
            if let Some(client) = server.clients.get_mut(client_uuid) {
                client.pending_responses.push(resp);
            }
            return;
        }
    };

    if !server.users.contains_key(&other_user_uuid) {
        let resp = Response {
            code: ResponseCode::Status(StatusCode::NotFound),
            data: Some(other_user_uuid),
        };
        if let Some(client) = server.clients.get_mut(client_uuid) {
            client.pending_responses.push(resp);
        }
        return;
    }
    let filtered_messages: Vec<_> = server
        .private_messages
        .iter()
        .filter(|m| {
            if let Location::USER = m.location_type {
                (m.sender_uuid == current_user_uuid && m.location_uuid == other_user_uuid)
                    || (m.sender_uuid == other_user_uuid && m.location_uuid == current_user_uuid)
            } else {
                false
            }
        })
        .collect();

    let mut data = String::new();
    for (i, m) in filtered_messages.iter().enumerate() {
        if i > 0 {
            data.push(' ');
        }
        let timestamp = m.date.to_timestamp();
        data.push_str(&format!(
            "\"{}\" \"{}\" \"{}\"",
            m.sender_uuid, timestamp, m.message
        ));
    }

    let resp = Response {
        code: ResponseCode::Status(StatusCode::Ok),
        data: if data.is_empty() { None } else { Some(data) },
    };
    if let Some(client) = server.clients.get_mut(client_uuid) {
        client.pending_responses.push(resp);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::{Date, Message, User};
    use std::collections::HashMap;
    use std::net::TcpListener;

    fn create_test_server() -> Server {
        Server {
            listener: TcpListener::bind("127.0.0.1:0").unwrap(),
            clients: HashMap::new(),
            users: HashMap::new(),
            private_messages: Vec::new(),
            teams: HashMap::new(),
        }
    }

    #[test]
    fn test_handle_messages_unauthorized() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            myteams::common::client::Client::new(socket),
        );

        handle_messages(&mut server, client_uuid, "other_user".to_string());

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            ResponseCode::Status(StatusCode::Unauthorized)
        );
    }

    #[test]
    fn test_handle_messages_not_found() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some("user1".to_string());
        server.clients.insert(client_uuid.to_string(), client);

        handle_messages(&mut server, client_uuid, "other_user".to_string());

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(
            client.pending_responses.last().unwrap().code,
            ResponseCode::Status(StatusCode::NotFound)
        );
    }

    #[test]
    fn test_handle_messages_ok() {
        let mut server = create_test_server();
        let client_uuid = "client1";
        let user1_uuid = "user1".to_string();
        let user2_uuid = "user2".to_string();

        server
            .users
            .insert(user1_uuid.clone(), User::new("u1".to_string()));
        server
            .users
            .insert(user2_uuid.clone(), User::new("u2".to_string()));

        let msg = Message {
            uuid: "m1".to_string(),
            message: "hi".to_string(),
            sender_uuid: user1_uuid.clone(),
            date: Date {
                day: 1,
                month: 1,
                year: 2024,
                hour: 10,
                minute: 0,
            },
            location_type: Location::USER,
            location_uuid: user2_uuid.clone(),
        };
        server.private_messages.push(msg);

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some(user1_uuid);
        server.clients.insert(client_uuid.to_string(), client);

        handle_messages(&mut server, client_uuid, user2_uuid);

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(
            client.pending_responses.last().unwrap().code,
            ResponseCode::Status(StatusCode::Ok)
        );
        assert!(
            client
                .pending_responses
                .last()
                .unwrap()
                .data
                .as_ref()
                .unwrap()
                .contains("hi")
        );
    }
}
