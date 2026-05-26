use crate::server::Server;
use myteams::common::message::{Location, Message};
use myteams::common::protocol::response::{Response, ResponseCode};
use myteams::common::protocol::status::StatusCode;
use myteams::common::utils::constants::MAX_BODY_LENGTH;
use myteams::common::utils::escape_str;
use myteams::server_event_private_message_sended;

pub fn handle_send(server: &mut Server, client_uuid: &str, receiver_uuid: String, body: String) {
    if body.len() > MAX_BODY_LENGTH {
        let resp = Response {
            code: ResponseCode::Status(StatusCode::BadRequest),
            data: None,
        };
        if let Some(client) = server.clients.get_mut(client_uuid) {
            client.pending_responses.push(resp);
        }
        return;
    }
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

    if !server.users.contains_key(&receiver_uuid) {
        let resp = Response {
            code: ResponseCode::Status(StatusCode::NotFound),
            data: Some(receiver_uuid),
        };
        if let Some(client) = server.clients.get_mut(client_uuid) {
            client.pending_responses.push(resp);
        }
        return;
    }

    let sender_user = server.users.get(&current_user_uuid).unwrap();
    let msg = Message::new(
        sender_user.uuid.clone(),
        Location::USER,
        receiver_uuid.clone(),
        body.clone(),
    );

    server.private_messages.push(msg);

    server_event_private_message_sended(&current_user_uuid, &receiver_uuid, &body);

    let resp = Response {
        code: ResponseCode::Status(StatusCode::Ok),
        data: None,
    };
    if let Some(client) = server.clients.get_mut(client_uuid) {
        client.pending_responses.push(resp);
    }

    let sender_uuid = current_user_uuid.clone();
    let event_resp = Response {
        code: ResponseCode::Event(myteams::common::protocol::event::EventCode::MessageReceived),
        data: Some(format!("\"{}\" \"{}\"", sender_uuid, escape_str(&body))),
    };

    server.send_to_user(&receiver_uuid, event_resp);
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::User;
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
    fn test_handle_send_body_too_long() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            myteams::common::client::Client::new(socket),
        );

        let long_body = "a".repeat(MAX_BODY_LENGTH + 1);
        handle_send(&mut server, client_uuid, "receiver1".to_string(), long_body);

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            ResponseCode::Status(StatusCode::BadRequest)
        );
    }

    #[test]
    fn test_handle_send_ok() {
        let mut server = create_test_server();
        let client1_uuid = "client1";
        let user1_uuid = "user1".to_string();
        let user2_uuid = "user2".to_string();

        server
            .users
            .insert(user1_uuid.clone(), User::new("u1".to_string()));
        server
            .users
            .insert(user2_uuid.clone(), User::new("u2".to_string()));

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some(user1_uuid.clone());
        server.clients.insert(client1_uuid.to_string(), client);

        handle_send(&mut server, client1_uuid, user2_uuid, "hello".to_string());

        assert_eq!(server.private_messages.len(), 1);
        assert_eq!(server.private_messages[0].message, "hello");
    }
}
