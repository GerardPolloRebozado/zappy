use crate::protocol::StatusCode;
use std::str::FromStr;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResponseCode {
    Status(StatusCode),
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
        }
    }
}
