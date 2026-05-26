use crate::client::Cli;
use myteams::common::protocol::ResponseCode;
use myteams::common::utils::parse_args;
use myteams::common::{Command, Request, StatusCode};
use myteams::{
    client_error_already_exist, client_error_unknown_team, client_print_subscribed,
    client_print_teams, client_print_users,
};
use std::io;

pub fn handle_subscribe_request(cli: &mut Cli, cmd: &str) -> io::Result<()> {
    let args = parse_args(cmd);
    if args.len() < 2 {
        return Err(io::Error::new(io::ErrorKind::Other, "Not enough arguments"));
    }
    let team_uuid = args[1].clone();
    cli.pending_request = Some(Request {
        command: Command::Subscribe(team_uuid),
    });
    if cli.handle_request().is_err() {
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Failed to send login request to server",
        ));
    }
    Ok(())
}

pub fn handle_subscribe_response(cli: &mut Cli) -> io::Result<()> {
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
                    client_print_subscribed(&parts[0], &parts[1]);
                }
            }
            return Ok(());
        }
        ResponseCode::Status(StatusCode::NotFound) => {
            if let Some(uuid) = response.data {
                client_error_unknown_team(&uuid);
            }
            return Ok(());
        }
        ResponseCode::Status(StatusCode::Conflict) => {
            client_error_already_exist();
            return Ok(());
        }
        _ => println!("Unexpected response from server: {:?}", response),
    }
    Err(io::Error::new(
        io::ErrorKind::Other,
        "Failed to handle user response",
    ))
}

pub fn handle_subscribed_request(cli: &mut Cli, cmd: &str) -> io::Result<()> {
    let args = parse_args(cmd);
    if args.len() > 1 {
        let team_uuid = args[1].clone();
        cli.pending_request = Some(Request {
            command: Command::Subscribed(Option::from(team_uuid)),
        });
    } else {
        cli.pending_request = Some(Request {
            command: Command::Subscribed(None),
        });
    }
    if cli.handle_request().is_err() {
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Failed to send login request to server",
        ));
    }
    Ok(())
}

pub fn handle_subscribed_response(cli: &mut Cli, cmd: &str) -> io::Result<()> {
    let response = cli.handle_response();
    if response.is_none() {
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Failed to read response from server",
        ));
    }
    let response = response.unwrap();
    let args = parse_args(cmd);
    let is_list_users = args.len() > 1;

    match response.code {
        ResponseCode::Status(StatusCode::Ok) => {
            if let Some(data) = response.data {
                let parsed_data = parse_args(&data);
                let mut i = 0;
                while i + 2 < parsed_data.len() {
                    if is_list_users {
                        let uuid = &parsed_data[i];
                        let name = &parsed_data[i + 1];
                        let status = parsed_data[i + 2].parse::<i32>().unwrap_or(0);
                        client_print_users(uuid, name, status);
                    } else {
                        let uuid = &parsed_data[i];
                        let name = &parsed_data[i + 1];
                        let desc = &parsed_data[i + 2];
                        client_print_teams(uuid, name, desc);
                    }
                    i += 3;
                }
            }
            return Ok(());
        }
        ResponseCode::Status(StatusCode::NotFound) => {
            if let Some(uuid) = response.data {
                client_error_unknown_team(&uuid);
            }
            return Ok(());
        }
        _ => println!("Unexpected response from server: {:?}", response),
    }
    Err(io::Error::new(
        io::ErrorKind::Other,
        "Failed to handle user response",
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;
    use std::net::TcpListener;

    #[test]
    fn test_handle_subscribe_response_ok() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 \"u1\" \"t1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_subscribe_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_subscribed_response_teams() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 \"t1\" \"n1\" \"d1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_subscribed_response(&mut cli, "/subscribed").unwrap();
    }

    #[test]
    fn test_handle_subscribed_response_users() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 \"u1\" \"n1\" \"1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_subscribed_response(&mut cli, "/subscribed \"t1\"").unwrap();
    }

    #[test]
    fn test_handle_subscribe_response_not_found() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"404 \"t1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_subscribe_response(&mut cli).unwrap();
    }
}
