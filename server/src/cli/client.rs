use crate::commands;
use crate::commands::handle_login_request;
use zappy::common::Command::Logout;
use zappy::common::utils::constants::MAX_BODY_LENGTH;
use zappy::common::{Request, Response, User};
use nix::poll::{PollFd, PollFlags};
use std::io;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::str::FromStr;

use std::collections::VecDeque;
use std::os::fd::AsFd;

pub struct Cli {
    pub socket: TcpStream,
    pub user: Option<User>,
    pub pending_request: Option<Request>,
    pub buffered_data: String,
    pub status_queue: VecDeque<Response>,
}

impl Cli {
    pub fn new(socket: &str) -> Self {
        let new_socket = TcpStream::connect(socket).expect("Failed to connect to server");
        Self {
            socket: new_socket,
            user: None,
            pending_request: None,
            buffered_data: String::new(),
            status_queue: VecDeque::new(),
        }
    }

    pub fn run(&mut self) {
        println!("Connected to server. Type /help for commands.");

        loop {
            let mut socket_ready = false;
            let mut socket_hup = false;
            let mut stdin_ready = false;

            {
                let stdin = io::stdin();
                let mut fds = vec![
                    PollFd::new(stdin.as_fd(), PollFlags::POLLIN),
                    PollFd::new(self.socket.as_fd(), PollFlags::POLLIN),
                ];

                if let Err(e) = nix::poll::poll(&mut fds, None::<u16>) {
                    if e == nix::errno::Errno::EINTR {
                        continue;
                    }
                    eprintln!("Poll error: {}", e);
                    break;
                }

                if let Some(r) = fds[1].revents() {
                    socket_ready = r.contains(PollFlags::POLLIN);
                    socket_hup = r.contains(PollFlags::POLLHUP);
                }
                if let Some(r) = fds[0].revents() {
                    stdin_ready = r.contains(PollFlags::POLLIN);
                }
            }

            // Handle Socket events (Priority)
            if socket_ready {
                if let Err(_) = self.process_socket_data() {
                    println!("Server disconnected.");
                    break;
                }
            } else if socket_hup {
                println!("Server connection lost.");
                break;
            }

            // Handle Stdin
            if stdin_ready {
                let mut input = String::new();
                if io::stdin().read_line(&mut input).is_ok() {
                    let trimmed = input.trim();
                    if trimmed.is_empty() {
                        continue;
                    }
                    if self.handle_command(trimmed).is_err() {
                        // handle_command already prints errors
                    }
                    if trimmed == "/logout" {
                        break;
                    }
                }
            }
        }
    }

    fn handle_command(&mut self, cmd: &str) -> io::Result<()> {
        let mut parts = cmd.trim().splitn(2, char::is_whitespace);
        let command = parts.next().unwrap_or("");

        match command {
            "/help" => {
                commands::help();
            }
            "/login" => {
                handle_login_request(self, cmd)?;
            }
            "/logout" => {
                self.pending_request = Some(Request { command: Logout });
                self.handle_request()?;
                std::process::exit(0);
            }
            _ => {
                println!("Unknown command: {}", command);
                return Err(io::Error::new(io::ErrorKind::Other, "Unknown command"));
            }
        }

        Ok(())
    }

    pub fn handle_request(&mut self) -> io::Result<()> {
        let req = match &self.pending_request {
            Some(r) => r.to_string().into_bytes(),
            None => return Ok(()),
        };

        while self.pending_request.is_some() {
            let mut socket_ready = false;
            {
                let mut fds = vec![PollFd::new(self.socket.as_fd(), PollFlags::POLLOUT)];
                if let Err(_e) = nix::poll::poll(&mut fds, None::<u16>) {
                    return Err(io::Error::new(io::ErrorKind::Other, "Poll error"));
                }
                if let Some(r) = fds[0].revents() {
                    socket_ready = r.contains(PollFlags::POLLOUT);
                }
            }

            if socket_ready {
                if let Err(_e) = self.socket.write_all(&req) {
                    return Err(io::Error::new(io::ErrorKind::Other, "Write error"));
                }
                self.pending_request = None;
                break;
            }
        }

        Ok(())
    }

    pub fn process_socket_data(&mut self) -> io::Result<()> {
        let mut buffer = [0; MAX_BODY_LENGTH];
        match self.socket.read(&mut buffer) {
            Ok(0) => {
                return Err(io::Error::new(
                    io::ErrorKind::ConnectionAborted,
                    "Server closed connection",
                ));
            }
            Ok(n) => {
                self.buffered_data
                    .push_str(&String::from_utf8_lossy(&buffer[..n]));
                while let Some(pos) = self.buffered_data.find('\n') {
                    let line = self.buffered_data[..pos].to_string();
                    self.buffered_data.drain(..pos + 1);
                    if line.trim().is_empty() {
                        continue;
                    }
                    if let Ok(response) = Response::from_str(line.trim()) {
                        match response.code {
                            zappy::common::protocol::response::ResponseCode::Event(_) => {
                                commands::handle_event(response);
                            }
                            zappy::common::protocol::response::ResponseCode::Status(_) => {
                                self.status_queue.push_back(response);
                            }
                        }
                    }
                }
            }
            Err(e) => return Err(e),
        }
        Ok(())
    }

    pub fn handle_response(&mut self) -> Option<Response> {
        loop {
            if let Some(resp) = self.status_queue.pop_front() {
                return Some(resp);
            }

            let mut socket_ready = false;
            {
                let mut fds = vec![PollFd::new(self.socket.as_fd(), PollFlags::POLLIN)];
                if let Err(_e) = nix::poll::poll(&mut fds, None::<u16>) {
                    return None;
                }
                if let Some(r) = fds[0].revents() {
                    socket_ready = r.contains(PollFlags::POLLIN);
                }
            }

            if socket_ready {
                if self.process_socket_data().is_err() {
                    return None;
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::net::TcpListener;

    #[test]
    fn test_cli_new() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();

        let _cli = Cli::new(&addr);
        assert!(_cli.user.is_none());
        assert!(_cli.status_queue.is_empty());
    }

    #[test]
    fn test_cli_process_socket_data() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();

        let mut cli = Cli::new(&addr);
        let (mut server_socket, _) = listener.accept().unwrap();

        server_socket.write_all(b"200 OK\n").unwrap();

        cli.process_socket_data().unwrap();
        assert_eq!(cli.status_queue.len(), 1);
        assert_eq!(cli.status_queue[0].to_string(), "200 OK\n");
    }

    #[test]
    fn test_cli_handle_response() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();

        let mut cli = Cli::new(&addr);
        let (mut server_socket, _) = listener.accept().unwrap();

        server_socket.write_all(b"200 Data\n").unwrap();

        let resp = cli.handle_response().unwrap();
        assert_eq!(resp.to_string(), "200 Data\n");
    }

    #[test]
    fn test_cli_handle_command_help() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap().to_string();
        let handle = std::thread::spawn(move || {
            let _ = listener.accept().unwrap();
        });
        let mut cli = Cli::new(&addr);
        handle.join().unwrap();
        cli.handle_command("/help").unwrap();
    }
}
