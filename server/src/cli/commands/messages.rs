use crate::client::Cli;
use myteams::common::protocol::command::Command;
use myteams::common::protocol::request::Request;
use myteams::common::protocol::response::ResponseCode;
use myteams::common::protocol::status::StatusCode;
use myteams::common::utils::parse_args;
use myteams::{
    client_error_unauthorized, client_error_unknown_user, client_private_message_print_messages,
};
use std::io;

pub fn handle_messages_request(cli: &mut Cli, cmd: &str) -> io::Result<()> {
    let other_user_uuid = match parse_messages_args(cmd) {
        Some(uuid) => uuid,
        None => {
            println!("Usage: /messages \"user_uuid\"");
            return Err(io::Error::new(
                io::ErrorKind::Other,
                "Invalid /messages arguments",
            ));
        }
    };

    cli.pending_request = Some(Request {
        command: Command::Messages(other_user_uuid),
    });
    cli.handle_request()
}

pub fn handle_messages_response(cli: &mut Cli) -> io::Result<()> {
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
        ResponseCode::Status(StatusCode::Ok) => {
            if let Some(data) = response.data {
                // Success: 200 ["sender_uuid" "timestamp" "message_body"]*
                parse_and_print_messages(&data);
            }
            Ok(())
        }
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

fn parse_messages_args(cmd: &str) -> Option<String> {
    let args = parse_args(cmd);
    if args.len() < 2 {
        return None;
    }
    Some(args[1].clone())
}

fn parse_and_print_messages(data: &str) {
    let args = parse_args(data);
    let mut i = 0;
    while i + 2 < args.len() {
        let sender_uuid = &args[i];
        let timestamp_str = &args[i + 1];
        let message_body = &args[i + 2];

        if let Ok(ts) = timestamp_str.parse::<i64>() {
            client_private_message_print_messages(sender_uuid, ts, message_body);
        }
        i += 3;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;
    use std::net::TcpListener;

    #[test]
    fn test_parse_messages_args() {
        assert_eq!(
            parse_messages_args("/messages \"uuid123\""),
            Some("uuid123".to_string())
        );
        assert_eq!(parse_messages_args("/messages"), None);
    }

    #[test]
    fn test_parse_and_print_messages() {
        let data = "\"sender1\" \"12345\" \"hello\" \"sender2\" \"67890\" \"world\"";
        // This shouldn't panic
        parse_and_print_messages(data);
    }

    #[test]
    fn test_handle_messages_response_ok() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 \"s1\" \"123\" \"m1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();
        handle_messages_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_messages_response_unauthorized() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"401\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();
        handle_messages_response(&mut cli).unwrap();
    }
}
