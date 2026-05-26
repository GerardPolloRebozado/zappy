use crate::server::Server;
use myteams::common::Response;
use myteams::common::StatusCode;
use myteams::common::protocol::ResponseCode;

pub fn handle_use(
    server: &mut Server,
    client_uuid: &str,
    team_uuid: Option<String>,
    channel_uuid: Option<String>,
    thread_uuid: Option<String>,
) {
    let client = match server.clients.get_mut(client_uuid) {
        Some(c) => c,
        None => return,
    };

    client.client_ctx.team_uuid = team_uuid;
    client.client_ctx.channel_uuid = channel_uuid;
    client.client_ctx.thread_uuid = thread_uuid;

    client.pending_responses.push(Response {
        code: ResponseCode::Status(StatusCode::Ok),
        data: None,
    });
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
            private_messages: Vec::new(),
            teams: HashMap::new(),
        }
    }

    #[test]
    fn test_handle_use_ok() {
        let mut server = create_test_server();
        let client_uuid = "client1";

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let socket = std::net::TcpStream::connect(listener.local_addr().unwrap()).unwrap();
        server.clients.insert(
            client_uuid.to_string(),
            myteams::common::client::Client::new(socket),
        );

        handle_use(
            &mut server,
            client_uuid,
            Some("team".to_string()),
            None,
            None,
        );

        let client = server.clients.get(client_uuid).unwrap();
        assert_eq!(client.client_ctx.team_uuid, Some("team".to_string()));
        assert_eq!(client.pending_responses.len(), 1);
        assert_eq!(
            client.pending_responses[0].code,
            ResponseCode::Status(StatusCode::Ok)
        );
    }
}
