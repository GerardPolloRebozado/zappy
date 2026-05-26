use crate::server::Server;
use myteams::common::protocol::ResponseCode::Status;
use myteams::common::{Response, StatusCode};
use myteams::server_event_user_subscribed;

pub fn handle_subscribe(server: &mut Server, client_uuid: &str, team_uuid: &str) {
    let client = server.clients.get_mut(client_uuid).unwrap();

    let team_to_subscribe = server.teams.get_mut(team_uuid);
    if team_to_subscribe.is_none() {
        client.pending_responses.push(Response::new(
            Status(StatusCode::NotFound),
            Some(team_uuid.to_string()),
        ));
        return;
    }

    let user_uuid = client.user.clone().unwrap();
    let team = team_to_subscribe.unwrap();
    if team.users_uuid.contains(&user_uuid) {
        client.pending_responses.push(Response::new(
            Status(StatusCode::Conflict),
            Some(client_uuid.to_string()),
        ));
        return;
    }
    team.users_uuid.push(user_uuid.to_string());
    client.pending_responses.push(Response::new(
        Status(StatusCode::Ok),
        Some(format!("\"{}\" \"{}\"", user_uuid, team.uuid)),
    ));
    server_event_user_subscribed(user_uuid.as_str(), team.uuid.as_str());
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::Team;
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
    fn test_handle_subscribe_not_found() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some("user1".to_string());
        server.clients.insert(client_uuid.to_string(), client);

        handle_subscribe(&mut server, client_uuid, "non-existent-team");

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            Status(StatusCode::NotFound)
        );
    }

    #[test]
    fn test_handle_subscribe_ok() {
        let mut server = create_test_server();
        let client_uuid = "client1";
        let team = Team::new("Team1".to_string());
        let team_uuid = team.uuid.clone();
        server.teams.insert(team_uuid.clone(), team);

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some("user1".to_string());
        server.clients.insert(client_uuid.to_string(), client);

        handle_subscribe(&mut server, client_uuid, &team_uuid);

        let team = server.teams.get(&team_uuid).unwrap();
        assert!(team.users_uuid.contains(&"user1".to_string()));
    }
}
