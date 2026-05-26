use crate::client::Cli;
use myteams::common::protocol::command::Command;
use myteams::common::protocol::request::Request;
use myteams::common::protocol::ResponseCode;
use myteams::common::utils::parse_args;
use myteams::common::StatusCode;
use myteams::{
    client_error_unauthorized, client_error_unknown_channel, client_error_unknown_team,
    client_error_unknown_thread, client_print_channel, client_print_team, client_print_thread,
    client_print_user,
};
use std::io;

pub fn handle_info_request(cli: &mut Cli, _cmd: &str) -> io::Result<()> {
    cli.pending_request = Some(Request {
        command: Command::Info,
    });

    cli.handle_request()
}

pub fn handle_info_response(cli: &mut Cli) -> io::Result<()> {
    let response = match cli.handle_response() {
        Some(r) => r,
        None => {
            return Err(io::Error::new(
                io::ErrorKind::Other,
                "Failed to read response from server",
            ));
        }
    };

    match response.code {
        ResponseCode::Status(StatusCode::Ok) => {
            if let Some(data) = response.data {
                let parts = parse_args(&data);
                if parts.is_empty() {
                    return Ok(());
                }

                let tag = &parts[0];
                let info = &parts[1..];

                match tag.as_str() {
                    "USER" => {
                        if info.len() >= 3 {
                            client_print_user(&info[0], &info[1], info[2].parse().unwrap_or(0));
                        }
                    }
                    "TEAM" => {
                        if info.len() >= 3 {
                            client_print_team(&info[0], &info[1], &info[2]);
                        }
                    }
                    "CHANNEL" => {
                        if info.len() >= 3 {
                            client_print_channel(&info[0], &info[1], &info[2]);
                        }
                    }
                    "THREAD" => {
                        if info.len() >= 5 {
                            client_print_thread(
                                &info[0],
                                &info[1],
                                info[2].parse().unwrap_or(0),
                                &info[3],
                                &info[4],
                            );
                        }
                    }
                    _ => println!("Unknown info resource: {}", tag),
                }
            }
            Ok(())
        }
        ResponseCode::Status(StatusCode::Unauthorized) => {
            client_error_unauthorized();
            Ok(())
        }
        ResponseCode::Status(StatusCode::NotFound) => {
            if let Some(data) = response.data {
                let parts = parse_args(&data);
                if parts.len() >= 2 {
                    let tag = &parts[0];
                    let uuid = &parts[1];
                    match tag.as_str() {
                        "TEAM" => client_error_unknown_team(uuid),
                        "CHANNEL" => client_error_unknown_channel(uuid),
                        "THREAD" => client_error_unknown_thread(uuid),
                        _ => {
                            println!("Resource not found: {} {}", tag, uuid);
                            0
                        }
                    };
                }
            }
            Ok(())
        }
        _ => {
            println!("Failed to get info: {:?}", response.code);
            Ok(())
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::Response;
    use std::net::TcpListener;
    use std::io::Write;

    #[test]
    fn test_handle_info_response_user() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 USER \"u\" \"n\" \"1\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_info_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_info_response_team() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 TEAM \"u\" \"n\" \"d\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_info_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_info_response_channel() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 CHANNEL \"u\" \"n\" \"d\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_info_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_info_response_thread() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"200 THREAD \"u\" \"us\" \"123\" \"t\" \"b\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_info_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_info_response_not_found() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"404 TEAM \"u\"\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_info_response(&mut cli).unwrap();
    }
}


