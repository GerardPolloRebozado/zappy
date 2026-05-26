use crate::server::Server;
use myteams::common::protocol::response::{Response, ResponseCode};
use myteams::common::protocol::status::StatusCode;

pub fn handle_list(server: &mut Server, client_uuid: &str) {
    let current_user_uuid = match get_authenticated_user(server, client_uuid) {
        Some(uuid) => uuid,
        None => {
            set_response(server, client_uuid, StatusCode::Unauthorized, None);
            return;
        }
    };

    let (team_ctx, channel_ctx, thread_ctx) = get_client_context(server, client_uuid);

    let (status, data) = match (team_ctx, channel_ctx, thread_ctx) {
        (None, None, None) => list_teams(server),
        (Some(t), None, None) => list_channels(server, &t, &current_user_uuid),
        (Some(t), Some(c), None) => list_threads(server, &t, &c, &current_user_uuid),
        (Some(t), Some(c), Some(th)) => list_replies(server, &t, &c, &th, &current_user_uuid),
        _ => (StatusCode::BadRequest, None),
    };

    set_response(server, client_uuid, status, data);
}

fn get_authenticated_user(server: &Server, client_uuid: &str) -> Option<String> {
    server.clients.get(client_uuid).and_then(|c| c.user.clone())
}

fn get_client_context(
    server: &Server,
    client_uuid: &str,
) -> (Option<String>, Option<String>, Option<String>) {
    let client = server.clients.get(client_uuid).unwrap();
    (
        client.client_ctx.team_uuid.clone(),
        client.client_ctx.channel_uuid.clone(),
        client.client_ctx.thread_uuid.clone(),
    )
}

fn set_response(server: &mut Server, client_uuid: &str, status: StatusCode, data: Option<String>) {
    let resp = Response {
        code: ResponseCode::Status(status),
        data,
    };

    if let Some(client) = server.clients.get_mut(client_uuid) {
        client.pending_responses.push(resp);
    }
}

fn list_teams(server: &Server) -> (StatusCode, Option<String>) {
    let mut data = String::from("TEAM");

    for team in server.teams.values() {
        data.push(' ');
        data.push_str(&format!(
            "\"{}\" \"{}\" \"{}\"",
            team.uuid, team.name, team.description
        ));
    }

    (StatusCode::Ok, Some(data))
}

fn list_channels(
    server: &Server,
    team_uuid: &str,
    _user_uuid: &str,
) -> (StatusCode, Option<String>) {
    let team = match server.teams.get(team_uuid) {
        Some(t) => t,
        None => {
            return (
                StatusCode::NotFound,
                Some(format!("TEAM \"{}\"", team_uuid)),
            );
        }
    };

    let mut data = String::from("CHANNEL");

    for channel in team.channels.values() {
        data.push(' ');
        data.push_str(&format!(
            "\"{}\" \"{}\" \"{}\"",
            channel.uuid, channel.name, channel.description
        ));
    }

    (StatusCode::Ok, Some(data))
}

fn list_threads(
    server: &Server,
    team_uuid: &str,
    channel_uuid: &str,
    _user_uuid: &str,
) -> (StatusCode, Option<String>) {
    if !server.teams.contains_key(team_uuid) {
        return (
            StatusCode::NotFound,
            Some(format!("TEAM \"{}\"", team_uuid)),
        );
    }
    let team = server.teams.get(team_uuid).unwrap();

    let channel = match team.channels.get(channel_uuid) {
        Some(c) => c,
        None => {
            return (
                StatusCode::NotFound,
                Some(format!("CHANNEL \"{}\"", channel_uuid)),
            );
        }
    };

    let mut data = String::from("THREAD");

    for thread in channel.threads.values() {
        data.push(' ');

        let (user_uuid, body) = thread
            .messages
            .first()
            .map(|m| (m.sender_uuid.clone(), m.message.clone()))
            .unwrap_or((String::new(), String::new()));

        data.push_str(&format!(
            "\"{}\" \"{}\" \"{}\" \"{}\" \"{}\"",
            thread.uuid,
            user_uuid,
            thread.creation_date.to_timestamp(),
            thread.title,
            body
        ));
    }

    (StatusCode::Ok, Some(data))
}

fn list_replies(
    server: &Server,
    team_uuid: &str,
    channel_uuid: &str,
    thread_uuid: &str,
    _user_uuid: &str,
) -> (StatusCode, Option<String>) {
    if !server.teams.contains_key(team_uuid) {
        return (
            StatusCode::NotFound,
            Some(format!("TEAM \"{}\"", team_uuid)),
        );
    }
    let team = server.teams.get(team_uuid).unwrap();

    if !team.channels.contains_key(channel_uuid) {
        return (
            StatusCode::NotFound,
            Some(format!("CHANNEL \"{}\"", channel_uuid)),
        );
    }
    let channel = team.channels.get(channel_uuid).unwrap();

    let thread = match channel.threads.get(thread_uuid) {
        Some(t) => t,
        None => {
            return (
                StatusCode::NotFound,
                Some(format!("THREAD \"{}\"", thread_uuid)),
            );
        }
    };

    let mut data = String::from("REPLY");

    for msg in thread.messages.iter() {
        data.push(' ');

        data.push_str(&format!(
            "\"{}\" \"{}\" \"{}\" \"{}\"",
            thread.uuid,
            msg.sender_uuid,
            msg.date.to_timestamp(),
            msg.message
        ));
    }

    (StatusCode::Ok, Some(data))
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::{Channel, Team};
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
    fn test_handle_list_unauthorized() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            myteams::common::client::Client::new(socket),
        );

        // No user set in client
        handle_list(&mut server, client_uuid);

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            ResponseCode::Status(StatusCode::Unauthorized)
        );
    }

    #[test]
    fn test_list_teams_ok() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        server
            .teams
            .insert("t1".to_string(), Team::new("Team1".to_string()));

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some("user-uuid".to_string());
        server.clients.insert(client_uuid.to_string(), client);

        handle_list(&mut server, client_uuid);

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
                .contains("TEAM")
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

    #[test]
    fn test_list_threads_ok() {
        use myteams::common::Thread;
        let mut server = create_test_server();
        let client_uuid = "client1";
        let user_uuid = "user-uuid".to_string();

        let mut team = Team::new("Team1".to_string());
        let mut channel = Channel::new("Chan1");
        channel
            .threads
            .insert("th1".to_string(), Thread::new("Thread1".to_string()));
        let team_uuid = team.uuid.clone();
        let channel_uuid = channel.uuid.clone();
        team.channels.insert(channel_uuid.clone(), channel);
        server.teams.insert(team_uuid.clone(), team);

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some(user_uuid);
        client.client_ctx.team_uuid = Some(team_uuid);
        client.client_ctx.channel_uuid = Some(channel_uuid);
        server.clients.insert(client_uuid.to_string(), client);

        handle_list(&mut server, client_uuid);

        let client = server.clients.get(client_uuid).unwrap();
        assert!(
            client
                .pending_responses
                .last()
                .unwrap()
                .data
                .as_ref()
                .unwrap()
                .contains("THREAD")
        );
    }

    #[test]
    fn test_list_replies_ok() {
        use myteams::common::message::Location;
        use myteams::common::{Date, Message, Thread};
        let mut server = create_test_server();
        let client_uuid = "client1";
        let user_uuid = "user-uuid".to_string();

        let mut team = Team::new("Team1".to_string());
        let mut channel = Channel::new("Chan1");
        let mut thread = Thread::new("Thread1".to_string());
        thread.messages.push(Message {
            uuid: "m1".to_string(),
            message: "reply1".to_string(),
            sender_uuid: user_uuid.clone(),
            date: Date {
                day: 1,
                month: 1,
                year: 2024,
                hour: 10,
                minute: 0,
            },
            location_type: Location::THREAD,
            location_uuid: "th1".to_string(),
        });

        let team_uuid = team.uuid.clone();
        let channel_uuid = channel.uuid.clone();
        let thread_uuid = thread.uuid.clone();
        channel.threads.insert(thread_uuid.clone(), thread);
        team.channels.insert(channel_uuid.clone(), channel);
        server.teams.insert(team_uuid.clone(), team);

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        let mut client = myteams::common::client::Client::new(socket);
        client.user = Some(user_uuid);
        client.client_ctx.team_uuid = Some(team_uuid);
        client.client_ctx.channel_uuid = Some(channel_uuid);
        client.client_ctx.thread_uuid = Some(thread_uuid);
        server.clients.insert(client_uuid.to_string(), client);

        handle_list(&mut server, client_uuid);

        let client = server.clients.get(client_uuid).unwrap();
        assert!(
            client
                .pending_responses
                .last()
                .unwrap()
                .data
                .as_ref()
                .unwrap()
                .contains("REPLY")
        );
    }
}
