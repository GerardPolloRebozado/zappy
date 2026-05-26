use crate::server::Server;
use myteams::common::protocol::ResponseCode::Status;
use myteams::common::{Response, StatusCode};

pub fn handle_subscribed_users(server: &mut Server, client_uuid: &str, team_uuid: String) {
    let team = server.teams.get(&team_uuid);
    if team.is_none() {
        let client = server.clients.get_mut(client_uuid).unwrap();
        client
            .pending_responses
            .push(Response::new(Status(StatusCode::NotFound), Some(team_uuid)));
        return;
    }
    let team = team.unwrap();
    let mut users_list = String::new();

    for user_uuid in &team.users_uuid {
        let user = server.users.get(user_uuid);
        if let Some(user) = user {
            let status = if server
                .clients
                .values()
                .any(|c| c.user.as_ref() == Some(&user.uuid))
            {
                1
            } else {
                0
            };
            users_list.push_str(&format!(
                "\"{}\" \"{}\" \"{}\" ",
                user.uuid, user.name, status
            ));
        }
    }
    let client = server.clients.get_mut(client_uuid).unwrap();
    client
        .pending_responses
        .push(Response::new(Status(StatusCode::Ok), Some(users_list)));
}

pub fn handle_subscribed_teams(server: &mut Server, client_uuid: &str) {
    let mut subscribed_teams = String::new();
    let client = server.clients.get(client_uuid).unwrap();
    let user_uuid = client.user.clone().unwrap();

    for team in server.teams.values() {
        if team.users_uuid.contains(&user_uuid) {
            subscribed_teams.push_str(&format!(
                "\"{}\" \"{}\" \"{}\" ",
                team.uuid, team.name, team.description
            ));
        }
    }
    let client = server.clients.get_mut(client_uuid).unwrap();
    client.pending_responses.push(Response::new(
        Status(StatusCode::Ok),
        Some(subscribed_teams),
    ));
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::{Team, User};
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
    fn test_handle_subscribed_users_not_found() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            myteams::common::client::Client::new(socket),
        );

        handle_subscribed_users(&mut server, client_uuid, "non-existent-team".to_string());

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            Status(StatusCode::NotFound)
        );
    }

    #[test]
    fn test_handle_subscribed_teams_ok() {
        let mut server = create_test_server();
        let client_uuid = "client1";
        let user_uuid = "user1".to_string();

        let mut team = Team::new("Team1".to_string());
        team.users_uuid.push(user_uuid.clone());
        server.teams.insert(team.uuid.clone(), team);

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some(user_uuid);
        server.clients.insert(client_uuid.to_string(), client);

        handle_subscribed_teams(&mut server, client_uuid);

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(
            client.pending_responses.last().unwrap().code,
            Status(StatusCode::Ok)
        );
        assert!(
            client
                .pending_responses
                .last()
                .unwrap()
                .data
                .as_ref()
                .unwrap()
                .contains("Team1")
        );
    }
}
