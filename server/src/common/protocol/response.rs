use crate::common::protocol::event::EventCode;
use crate::common::protocol::status::StatusCode;
use std::str::FromStr;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResponseCode {
    Status(StatusCode),
    Event(EventCode),
}

#[derive(Debug, Clone, PartialEq)]
pub struct Response {
    pub code: ResponseCode,
    pub data: Option<String>,
}

impl Response {
    pub fn new(code: ResponseCode, data: Option<String>) -> Self {
        Self { code, data }
    }
}

impl FromStr for Response {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let s = s.trim();
        if s.is_empty() {
            return Err(());
        }

        // Zappy specific logic: 
        // "ok" -> Status(StatusCode::Ok)
        // "ko" -> Status(StatusCode::Ko)
        // Everything else is either data or an event
        if s == "ok" {
            return Ok(Response { code: ResponseCode::Status(StatusCode::Ok), data: None });
        }
        if s == "ko" {
            return Ok(Response { code: ResponseCode::Status(StatusCode::Ko), data: None });
        }

        // For now, let's treat other strings as Status::Ok with data if they don't match events
        // In a real implementation, we'd have a more robust parser for GUI events (msz, bct, etc)
        Ok(Response {
            code: ResponseCode::Status(StatusCode::Ok),
            data: Some(s.to_string()),
        })
    }
}

impl ToString for Response {
    fn to_string(&self) -> String {
        match self.code {
            ResponseCode::Status(StatusCode::Ok) => {
                if let Some(ref data) = self.data {
                    format!("{}\n", data)
                } else {
                    "ok\n".to_string()
                }
            }
            ResponseCode::Status(StatusCode::Ko) => "ko\n".to_string(),
            ResponseCode::Event(_) => {
                if let Some(ref data) = self.data {
                    format!("{}\n", data)
                } else {
                    "\n".to_string()
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::common::protocol::status::StatusCode;

    #[test]
    fn test_response_to_string_ok() {
        let resp = Response {
            code: ResponseCode::Status(StatusCode::Ok),
            data: None,
        };
        assert_eq!(resp.to_string(), "ok\n");
    }

    #[test]
    fn test_response_to_string_ko() {
        let resp = Response {
            code: ResponseCode::Status(StatusCode::Ko),
            data: None,
        };
        assert_eq!(resp.to_string(), "ko\n");
    }

    #[test]
    fn test_response_to_string_data() {
        let resp = Response {
            code: ResponseCode::Status(StatusCode::Ok),
            data: Some("msz 10 10".to_string()),
        };
        assert_eq!(resp.to_string(), "msz 10 10\n");
    }

    #[test]
    fn test_response_from_str_ok() {
        let resp = Response::from_str("ok").unwrap();
        assert_eq!(resp.code, ResponseCode::Status(StatusCode::Ok));
    }
}
