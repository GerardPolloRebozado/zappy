use crate::server::Server;
use myteams::common::protocol::ResponseCode;
use myteams::common::{Response, StatusCode};
use myteams::server_event_user_unsubscribed;

pub fn handle_unsubscribe(server: &mut Server, client_uuid: &str, team_uuid: &str) {
    let client = server.clients.get_mut(client_uuid);
    if client.is_none() {
        return;
    }
    let client = client.unwrap();
    let team = server.teams.get_mut(team_uuid);
    if team.is_none() {
        client.pending_responses.push(Response::new(
            ResponseCode::Status(StatusCode::NotFound),
            Some(format!("Team: {} not found", team_uuid)),
        ));
        return;
    }
    let team = team.unwrap();
    let user_uuid = client.user.clone().unwrap();
    let user_index = team.users_uuid.iter().position(|u| u == &user_uuid);
    if user_index.is_none() {
        client.pending_responses.push(Response::new(
            ResponseCode::Status(StatusCode::NotFound),
            Some(format!(
                "User: {} in team: {} not found",
                user_uuid, team_uuid
            )),
        ));
        return;
    }
    team.users_uuid.remove(user_index.unwrap());
    client.pending_responses.push(Response::new(
        ResponseCode::Status(StatusCode::Ok),
        Some(format!("\"{}\" \"{}\"", user_uuid, team.uuid)),
    ));
    server_event_user_unsubscribed(user_uuid.as_str(), team.uuid.as_str());
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
    fn test_handle_unsubscribe_not_found() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            myteams::common::client::Client::new(socket),
        );

        handle_unsubscribe(&mut server, client_uuid, "non-existent-team");

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            ResponseCode::Status(StatusCode::NotFound)
        );
    }

    #[test]
    fn test_handle_unsubscribe_ok() {
        let mut server = create_test_server();
        let client_uuid = "client1";
        let user_uuid = "user1".to_string();

        let mut team = Team::new("Team1".to_string());
        team.users_uuid.push(user_uuid.clone());
        let team_uuid = team.uuid.clone();
        server.teams.insert(team_uuid.clone(), team);

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some(user_uuid.clone());
        server.clients.insert(client_uuid.to_string(), client);

        handle_unsubscribe(&mut server, client_uuid, &team_uuid);

        let team = server.teams.get(&team_uuid).unwrap();
        assert!(!team.users_uuid.contains(&user_uuid));
    }
}
