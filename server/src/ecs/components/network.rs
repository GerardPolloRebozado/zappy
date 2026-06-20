use crate::protocol::Response;
use crate::utils::constants::MAX_BODY_LENGTH;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::os::fd::AsFd;

pub trait Socket: Read + Write + AsFd + Send {}
impl Socket for TcpStream {}

pub struct NetworkData {
    pub socket: Option<Box<dyn Socket>>,
    pub user: Option<String>,
    buffered_data: Option<String>,
    pub pending_responses: Vec<Response>,
    pub incoming_commands: Vec<String>,
}

impl NetworkData {
    pub fn new<S: Socket + 'static>(socket: S) -> NetworkData {
        NetworkData {
            socket: Some(Box::new(socket)),
            user: None,
            buffered_data: None,
            pending_responses: Vec::new(),
            incoming_commands: Vec::new(),
        }
    }

    pub fn new_headless() -> NetworkData {
        NetworkData {
            socket: None,
            user: None,
            buffered_data: None,
            pending_responses: Vec::new(),
            incoming_commands: Vec::new(),
        }
    }

    pub fn read_data(&mut self) -> Option<String> {
        let socket = self.socket.as_mut()?;
        let mut buffer = [0; MAX_BODY_LENGTH];
        match socket.read(&mut buffer) {
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

/// Mock socket for testing purposes, it uses an in-memory buffer for input and output, and a dummy file descriptor to satisfy the AsFd trait
#[cfg(test)]
pub struct MockSocket {
    pub input: std::io::Cursor<Vec<u8>>,
    pub output: std::sync::Arc<std::sync::Mutex<Vec<u8>>>,
    _dummy_file: std::fs::File,
}

#[cfg(test)]
impl MockSocket {
    pub fn new(input: Vec<u8>) -> (Self, std::sync::Arc<std::sync::Mutex<Vec<u8>>>) {
        let output = std::sync::Arc::new(std::sync::Mutex::new(Vec::new()));
        (
            Self {
                input: std::io::Cursor::new(input),
                output: output.clone(),
                _dummy_file: std::fs::File::open("/dev/null").expect("failed to open /dev/null"),
            },
            output,
        )
    }
}

#[cfg(test)]
impl Read for MockSocket {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        self.input.read(buf)
    }
}

#[cfg(test)]
impl Write for MockSocket {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.output.lock().unwrap().write(buf)
    }
    fn flush(&mut self) -> std::io::Result<()> {
        self.output.lock().unwrap().flush()
    }
}

#[cfg(test)]
impl AsFd for MockSocket {
    fn as_fd(&self) -> std::os::fd::BorrowedFd<'_> {
        self._dummy_file.as_fd()
    }
}

#[cfg(test)]
impl Socket for MockSocket {}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_read_data() {
        let (mock_socket, _) = MockSocket::new(b"hello\n".to_vec());
        let mut nd = NetworkData::new(mock_socket);
        let msg = nd.read_data().unwrap();
        assert_eq!(msg, "hello\n");
    }

    #[test]
    fn test_read_data_partial() {
        let (mock_socket, _) = MockSocket::new(b"hel".to_vec());
        let mut nd = NetworkData::new(mock_socket);
        let msg = nd.read_data();
        assert!(msg.is_none());

        let (mock_socket, _) = MockSocket::new(b"lo\n".to_vec());
        nd.socket = Some(Box::new(mock_socket));
        let msg = nd.read_data().unwrap();
        assert_eq!(msg, "hello\n");
    }
}
