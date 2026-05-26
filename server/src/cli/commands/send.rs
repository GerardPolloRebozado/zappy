use crate::client::Cli;
use myteams::common::protocol::command::Command;
use myteams::common::protocol::request::Request;
use myteams::common::protocol::response::ResponseCode;
use myteams::common::protocol::status::StatusCode;
use myteams::common::utils::constants::MAX_BODY_LENGTH;
use myteams::common::utils::parse_args;
use myteams::{client_error_unauthorized, client_error_unknown_user};
use std::io;

pub fn handle_send_request(cli: &mut Cli, cmd: &str) -> io::Result<()> {
    let args = parse_send_args(cmd);
    let (receiver_uuid, message_body) = match args {
        Some((u, m)) => (u, m),
        None => {
            println!("Usage: /send \"user_uuid\" \"message_body\"");
            return Err(io::Error::new(
                io::ErrorKind::Other,
                "Invalid /send arguments",
            ));
        }
    };

    if message_body.len() > MAX_BODY_LENGTH {
        println!("Error: Message body too long (max {})", MAX_BODY_LENGTH);
        return Err(io::Error::new(io::ErrorKind::Other, "Message too long"));
    }

    cli.pending_request = Some(Request {
        command: Command::Send(receiver_uuid, message_body),
    });
    cli.handle_request()
}

pub fn handle_send_response(cli: &mut Cli) -> io::Result<()> {
    let response = match cli.handle_response() {
        Some(r) => r,
        None => {
            return Err(io::Error::new(
                io::ErrorKind::Other,
                "Failed to read response",
            ));
        }
    };

    match response.code {
        ResponseCode::Status(StatusCode::Ok) => Ok(()),
        ResponseCode::Status(StatusCode::Unauthorized) => {
            client_error_unauthorized();
            Ok(())
        }
        ResponseCode::Status(StatusCode::NotFound) => {
            if let Some(uuid) = response.data {
                client_error_unknown_user(&uuid);
            }
            Ok(())
        }
        _ => {
            println!("Unexpected response: {:?}", response);
            Err(io::Error::new(io::ErrorKind::Other, "Unexpected response"))
        }
    }
}

fn parse_send_args(cmd: &str) -> Option<(String, String)> {
    let args = parse_args(cmd);

    if args.len() >= 3 {
        Some((args[1].clone(), args[2].clone()))
    } else {
        None
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;
    use std::net::TcpListener;

    #[test]
    fn test_parse_send_args_valid() {
        let args = parse_send_args(r#"/send "user-uuid" "Hello, world!""#);
        assert!(args.is_some());
        let (uuid, body) = args.unwrap();
        assert_eq!(uuid, "user-uuid");
        assert_eq!(body, "Hello, world!");
    }

    #[test]
    fn test_handle_send_response_ok() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 OK\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_send_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_send_response_not_found() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"404 \"u1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_send_response(&mut cli).unwrap();
    }
}
