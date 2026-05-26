use crate::server::Server;
use zappy::common::protocol::ResponseCode;
use zappy::common::{Response, StatusCode};

pub fn handle_users(server: &mut Server, client_uuid: &str) {
    let mut users_list = "".to_string();

    for user in server.users.values() {
        let status = if server.clients.values().any(|c| c.user.as_ref() == Some(&user.uuid)) {
            1
        } else {
            0
        };
        users_list.push_str(&format!("\"{}\" \"{}\" \"{}\" ", user.uuid, user.name, status));
    }
    let client = server.clients.get_mut(client_uuid).unwrap();
    client.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some(users_list),
    ));
}

pub fn handle_user(server: &mut Server, client_uuid: &str, user_uuid: &str) {
    let user = server.users.get(user_uuid);
    if user.is_none() {
        let client = server.clients.get_mut(client_uuid).unwrap();
        client.pending_responses.push(Response::new(
            ResponseCode::Status(StatusCode::NotFound),
            Some(user_uuid.to_string()),
        ));
        return;
    }
    let user = user.unwrap();
    let status = if server.clients.values().any(|c| c.user.as_ref() == Some(&user.uuid)) {
        1
    } else {
        0
    };
    let data = format!("\"{}\" \"{}\" \"{}\"", user.uuid, user.name, status);
    let client = server.clients.get_mut(client_uuid).unwrap();
    client.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some(data),
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;
    use std::net::TcpListener;
    use zappy::common::User;

    fn create_test_server() -> Server {
        Server {
            listener: TcpListener::bind("127.0.0.1:0").unwrap(),
            clients: HashMap::new(),
            users: HashMap::new(),
            teams: HashMap::new(),
        }
    }

    #[test]
    fn test_handle_user_not_found() {
        let mut server = create_test_server();
        let client_uuid = "client1";
        
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(client_uuid.to_string(), zappy::common::client::Client::new(socket));
        
        handle_user(&mut server, client_uuid, "non-existent-user");
        
        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(client.pending_responses[0].code, ResponseCode::Status(StatusCode::NotFound));
    }

    #[test]
    fn test_handle_users_ok() {
        let mut server = create_test_server();
        let client_uuid = "client1";
        server.users.insert("u1".to_string(), User::new("user1".to_string()));

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let client = zappy::common::client::Client::new(socket);
        server.clients.insert(client_uuid.to_string(), client);
        
        handle_users(&mut server, client_uuid);
        
        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.last().unwrap().code, ResponseCode::Status(StatusCode::Ok));
        assert!(client.pending_responses.last().unwrap().data.as_ref().unwrap().contains("user1"));
    }
}
