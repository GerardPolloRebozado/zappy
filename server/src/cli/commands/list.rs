use crate::client::Cli;
use myteams::common::StatusCode;
use myteams::common::protocol::ResponseCode;
use myteams::common::protocol::command::Command;
use myteams::common::protocol::request::Request;
use myteams::common::utils::parse_args;
use myteams::{
    client_channel_print_threads, client_error_unauthorized, client_error_unknown_channel,
    client_error_unknown_team, client_error_unknown_thread, client_print_teams,
    client_team_print_channels, client_thread_print_replies,
};
use std::io;

pub fn handle_list_request(cli: &mut Cli, _cmd: &str) -> io::Result<()> {
    cli.pending_request = Some(Request {
        command: Command::List,
    });

    cli.handle_request()
}

pub fn handle_list_response(cli: &mut Cli) -> io::Result<()> {
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
                    "TEAM" => {
                        let mut i = 0;
                        while i + 2 < info.len() {
                            client_print_teams(&info[i], &info[i + 1], &info[i + 2]);
                            i += 3;
                        }
                    }
                    "CHANNEL" => {
                        let mut i = 0;
                        while i + 2 < info.len() {
                            client_team_print_channels(&info[i], &info[i + 1], &info[i + 2]);
                            i += 3;
                        }
                    }
                    "THREAD" => {
                        let mut i = 0;
                        while i + 4 < info.len() {
                            client_channel_print_threads(
                                &info[i],
                                &info[i + 1],
                                info[i + 2].parse().unwrap_or(0),
                                &info[i + 3],
                                &info[i + 4],
                            );
                            i += 5;
                        }
                    }
                    "REPLY" => {
                        let mut i = 0;
                        while i + 3 < info.len() {
                            client_thread_print_replies(
                                &info[i],
                                &info[i + 1],
                                info[i + 2].parse().unwrap_or(0),
                                &info[i + 3],
                            );
                            i += 4;
                        }
                    }
                    _ => println!("Unknown list resource: {}", tag),
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
                } else if !data.is_empty() {
                    println!("Resource not found: {}", data);
                }
            }
            Ok(())
        }
        _ => {
            println!("Failed to list resources: {:?}", response.code);
            Ok(())
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use myteams::common::Response;
    use std::io::Write;
    use std::net::TcpListener;

    #[test]
    fn test_handle_list_response_teams() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket
                .write_all(b"200 TEAM \"u1\" \"n1\" \"d1\" \"u2\" \"n2\" \"d2\"\n")
                .unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_list_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_list_response_channels() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket
                .write_all(b"200 CHANNEL \"u1\" \"n1\" \"d1\"\n")
                .unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_list_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_list_response_threads() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket
                .write_all(b"200 THREAD \"u1\" \"us\" \"123\" \"t1\" \"b1\"\n")
                .unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_list_response(&mut cli).unwrap();
    }

    #[test]
    fn test_handle_list_response_replies() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket
                .write_all(b"200 REPLY \"th1\" \"u1\" \"123\" \"b1\"\n")
                .unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();

        handle_list_response(&mut cli).unwrap();
    }
}
