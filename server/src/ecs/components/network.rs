use crate::protocol::Response;
use crate::utils::constants::MAX_BODY_LENGTH;
use std::io::Read;
use std::net::TcpStream;

#[derive(Clone, PartialEq, Eq)]
pub enum ClientState {
    WaitingForTeamName,
    AuthenticatedAI(String),
    AuthenticatedGUI,
}

pub struct NetworkData {
    pub socket: TcpStream,
    pub user: Option<String>,
    buffered_data: Option<String>,
    pub pending_responses: Vec<Response>,
    pub state: ClientState,
}

impl NetworkData {
    pub fn new(socket: TcpStream) -> NetworkData {
        NetworkData {
            socket,
            user: None,
            buffered_data: None,
            pending_responses: Vec::new(),
            state: ClientState::WaitingForTeamName,
        }
    }

    pub fn read_data(&mut self) -> Option<String> {
        let mut buffer = [0; MAX_BODY_LENGTH];
        match self.socket.read(&mut buffer) {
            Ok(0) => Some(String::from("/disconnect")),
            Ok(n) => {
                let msg = String::from_utf8_lossy(&buffer[..n]);

                let mut buf = self.buffered_data.take().unwrap_or_default();
                buf.push_str(&msg);

                if buf.ends_with('\n') {
                    Some(buf)
                } else {
                    self.buffered_data = Some(buf);
                    None
                }
            }
            Err(_) => None,
        }
    }
}
