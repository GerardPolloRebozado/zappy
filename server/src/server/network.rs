use crate::protocol::{Request, Response, ResponseCode, StatusCode};
use crate::server::Server;
use crate::server::client::Client;
use crate::server::commands;
use nix::poll::PollFlags;
use std::io::Write;

pub fn accept_connections(server: &mut Server) {
    if let Ok((mut socket, _addr)) = server.listener.accept() {
        let _ = socket.write_all(b"WELCOME\n");
        let new_client = Client::new(socket);
        server.clients.insert(new_client.uuid.clone(), new_client);
    }
}

pub fn process_client_events(
    server: &mut Server,
    client_revents: Vec<PollFlags>,
    client_keys: Vec<String>,
) {
    let mut disconnected = Vec::new();
    let mut requests_to_read = Vec::new();
    let mut requests_to_write = Vec::new();

    for (i, uuid) in client_keys.into_iter().enumerate() {
        let revents = client_revents[i];

        if revents.contains(PollFlags::POLLOUT) {
            requests_to_write.push(uuid.clone());
        }

        if !revents.contains(PollFlags::POLLIN) {
            continue;
        }

        let client = match server.clients.get_mut(&uuid) {
            Some(c) => c,
            None => continue,
        };

        match client.read_data() {
            Some(msg) => requests_to_read.push((uuid.clone(), msg)),
            None => disconnected.push(uuid.clone()),
        }
    }

    for (uuid, msg) in requests_to_read {
        for line in msg.split('\n') {
            let trimmed = line.trim();
            if trimmed.is_empty() {
                continue;
            }

            let req = match trimmed.parse::<Request>() {
                Ok(req) => req,
                Err(_) => {
                    if let Some(client) = server.clients.get_mut(&uuid) {
                        client.pending_responses.push(Response {
                            code: ResponseCode::Status(StatusCode::Ko),
                            data: None,
                        });
                    }
                    continue;
                }
            };

            commands::handle_request(server, &uuid, req);
        }
    }

    for uuid in requests_to_write {
        let client = server.clients.get_mut(&uuid);
        if let Some(c) = client {
            let responses: Vec<Response> = c.pending_responses.drain(..).collect();
            for response in responses {
                server.handle_response(uuid.as_str(), response);
            }
        }
    }

    for uuid in disconnected {
        if let Some(_client) = server.clients.remove(&uuid) {
            // TODO: decide what to do when a user disconnects
        }
    }
}
