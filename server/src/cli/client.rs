use nix::poll::{PollFd, PollFlags};
use std::io;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::os::fd::AsFd;

pub struct Cli {
    pub socket: TcpStream,
}

impl Cli {
    pub fn new(socket: &str) -> Self {
        let new_socket = TcpStream::connect(socket).expect("Failed to connect to server");
        Self { socket: new_socket }
    }

    pub fn run(&mut self) {
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

            // Read from server and print to stdout
            if socket_ready {
                let mut buffer = [0; 4096];
                match self.socket.read(&mut buffer) {
                    Ok(0) => break,
                    Ok(n) => {
                        let _ = io::stdout().write_all(&buffer[..n]);
                        let _ = io::stdout().flush();
                    }
                    Err(_) => break,
                }
            } else if socket_hup {
                break;
            }

            // Read from stdin and send to server
            if stdin_ready {
                let mut input = String::new();
                if io::stdin().read_line(&mut input).is_ok() {
                    if self.socket.write_all(input.as_bytes()).is_err() {
                        break;
                    }
                }
            }
        }
    }
}
