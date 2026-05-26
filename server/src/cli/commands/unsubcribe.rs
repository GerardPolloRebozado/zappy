use crate::client::Cli;
use myteams::common::utils::parse_args;
use myteams::common::{Command, Request};
use myteams::client_print_unsubscribed;
use std::io;

pub fn handle_unsubscribe_request(cli: &mut Cli, cmd: &str) -> io::Result<()> {
    let args = parse_args(cmd);
    if args.len() != 2 {
        println!("Usage: /unsubscribe <team_uuid>");
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Not enough arguments for unsubscribe command",
        ));
    }
    cli.pending_request = Some(Request {
        command: Command::Unsubscribe(args[1].clone()),
    });
    if cli.handle_request().is_err() {
        println!("Failed to send unsubscribe request to server");
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Failed to send login request to server",
        ));
    }
    Ok(())
}

pub fn handle_unsubscribe_response(cli: &mut Cli) -> io::Result<()> {
    let response = cli.handle_response();
    if response.is_none() {
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Failed to read response from server",
        ));
    }
    let response = response.unwrap();
    match response.code {
        myteams::common::protocol::ResponseCode::Status(myteams::common::StatusCode::Ok) => {
            if let Some(data) = response.data {
                let parts = parse_args(&data);
                if parts.len() >= 2 {
                    client_print_unsubscribed(&parts[0], &parts[1]);
                }
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
    use std::net::TcpListener;
    use std::io::Write;

    #[test]
    fn test_handle_unsubscribe_response_unexpected() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            socket.write_all(b"404 Not Found\n").unwrap();
        });

        let mut cli = Cli::new(&addr);
        handle.join().unwrap();
        
        assert!(handle_unsubscribe_response(&mut cli).is_err());
    }
}





