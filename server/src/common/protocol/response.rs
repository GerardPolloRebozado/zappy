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

        if s == "ok" {
            return Ok(Response {
                code: ResponseCode::Status(StatusCode::Ok),
                data: None,
            });
        }
        if s == "ko" {
            return Ok(Response {
                code: ResponseCode::Status(StatusCode::Ko),
                data: None,
            });
        }

        let parts: Vec<&str> = s.split_whitespace().collect();
        let prefix = parts[0];

        let event = match prefix {
            "message" => Some(EventCode::Message),
            "eject:" => Some(EventCode::Eject),
            "dead" => Some(EventCode::Dead),
            "pnw" => Some(EventCode::Pnw),
            "pdi" => Some(EventCode::Pdi),
            "ppo" => Some(EventCode::Ppo),
            "pie" => Some(EventCode::Pie),
            "seg" => Some(EventCode::Seg),
            _ => None,
        };

        if let Some(e) = event {
            return Ok(Response {
                code: ResponseCode::Event(e),
                data: if parts.len() > 1 {
                    Some(parts[1..].join(" "))
                } else {
                    None
                },
            });
        }

        Ok(Response {
            code: ResponseCode::Status(StatusCode::Ok),
            data: Some(s.to_string()),
        })
    }
}

impl std::fmt::Display for Response {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.code {
            ResponseCode::Status(StatusCode::Ok) => {
                if let Some(ref data) = self.data {
                    writeln!(f, "{}", data)
                } else {
                    writeln!(f, "ok")
                }
            }
            ResponseCode::Status(StatusCode::Ko) => writeln!(f, "ko"),
            ResponseCode::Event(e) => {
                let prefix = match e {
                    EventCode::Message => "message",
                    EventCode::Eject => "eject:",
                    EventCode::Dead => "dead",
                    EventCode::Pnw => "pnw",
                    EventCode::Pdi => "pdi",
                    EventCode::Ppo => "ppo",
                    EventCode::Pie => "pie",
                    EventCode::Seg => "seg",
                };
                if let Some(ref data) = self.data {
                    writeln!(f, "{} {}", prefix, data)
                } else {
                    writeln!(f, "{}", prefix)
                }
            }
        }
    }
}
