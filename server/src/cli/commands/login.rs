use crate::client::Cli;
use myteams::common::StatusCode;
use myteams::common::protocol::ResponseCode;
use myteams::common::protocol::command::Command;
use myteams::common::protocol::request::Request;
use myteams::common::utils::constants::MAX_NAME_LENGTH;
use myteams::common::utils::parse_args;
use myteams::{client_error_unauthorized, common};
use std::io;

pub fn handle_login_request(cli: &mut Cli, cmd: &str) -> io::Result<()> {
    let username = match parse_username(cmd) {
        Some(name) => name,
        None => {
            return Err(io::Error::new(
                io::ErrorKind::Other,
                "Invalid username format",
            ));
        }
    };

    if username.len() > MAX_NAME_LENGTH {
        println!("Error: Username too long (max {})", MAX_NAME_LENGTH);
        return Err(io::Error::new(io::ErrorKind::Other, "Username too long"));
    }

    cli.pending_request = Some(Request {
        command: Command::Login(username.to_string()),
    });
    if cli.handle_request().is_err() {
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Failed to send login request to server",
        ));
    }
    Ok(())
}

pub fn handle_login_response(cli: &mut Cli) -> io::Result<()> {
    let response = cli.handle_response();
    if response.is_none() {
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Failed to read response from server",
        ));
    }
    let response = response.unwrap();
    match response.code {
        ResponseCode::Status(StatusCode::Ok) => {
            if let Some(data) = response.data {
                let parts = parse_args(&data);
                if parts.len() >= 2 {
                    let uuid = &parts[0];
                    let name = &parts[1];
                    cli.user = Some(common::User {
                        uuid: uuid.clone(),
                        name: name.clone(),
                    });
                }
            }
            return Ok(());
        }
        ResponseCode::Status(StatusCode::Unauthorized) => {
            client_error_unauthorized();
            return Ok(());
        }
        _ => println!("Unexpected response from server: {:?}", response),
    }
    Err(io::Error::new(
        io::ErrorKind::Other,
        "Failed to handle login response",
    ))
}

fn parse_username(cmd: &str) -> Option<String> {
    let args = parse_args(cmd);
    if args.len() < 2 {
        println!("Usage: /login \"user_name\"");
        return None;
    }
    Some(args[1].clone())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;
    use std::net::TcpListener;

    #[test]
    fn test_handle_login_request_ok() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 OK\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle_login_request(&mut cli, "/login \"alex\"").unwrap();
        handle.join().unwrap();
    }

    #[test]
    fn test_handle_login_response_ok() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 \"u1\" \"n1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();
        handle_login_response(&mut cli).unwrap();
        assert!(cli.user.is_some());
        assert_eq!(cli.user.unwrap().name, "n1");
    }

    #[test]
    fn test_handle_login_response_unauthorized() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"401\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();
        handle_login_response(&mut cli).unwrap();
    }
}
