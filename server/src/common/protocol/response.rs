use crate::common::protocol::event::EventCode;
use crate::common::protocol::status::StatusCode;
use std::str::FromStr;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResponseCode {
    Status(StatusCode),
    Event(EventCode),
}

impl ResponseCode {
    pub fn to_u32(&self) -> u32 {
        match self {
            ResponseCode::Status(s) => *s as u32,
            ResponseCode::Event(e) => *e as u32,
        }
    }
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
    // TODO: Use a custom ProtocolError enum here to differentiate between
    // "Not a number" and "Unknown status code" during server response parsing.
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let parts: Vec<&str> = s.split_whitespace().collect();
        if parts.is_empty() {
            return Err(());
        }

        let code_u32 = parts[0].parse::<u32>().map_err(|_| ())?;
        let code = match code_u32 {
            200 => ResponseCode::Status(StatusCode::Ok),
            201 => ResponseCode::Status(StatusCode::Created),
            400 => ResponseCode::Status(StatusCode::BadRequest),
            401 => ResponseCode::Status(StatusCode::Unauthorized),
            403 => ResponseCode::Status(StatusCode::Forbidden),
            404 => ResponseCode::Status(StatusCode::NotFound),
            409 => ResponseCode::Status(StatusCode::Conflict),
            442 => ResponseCode::Status(StatusCode::ImACoffeePot),
            500 => ResponseCode::Status(StatusCode::InternalServerError),
            600 => ResponseCode::Event(EventCode::LoggedIn),
            601 => ResponseCode::Event(EventCode::LoggedOut),
            602 => ResponseCode::Event(EventCode::MessageReceived),
            603 => ResponseCode::Event(EventCode::ThreadCreated),
            604 => ResponseCode::Event(EventCode::CommentCreated),
            605 => ResponseCode::Event(EventCode::TeamCreated),
            606 => ResponseCode::Event(EventCode::ChannelCreated),
            _ => return Err(()),
        };

        let data = if parts.len() > 1 {
            Some(parts[1..].join(" "))
        } else {
            None
        };

        Ok(Response { code, data })
    }
}

impl ToString for Response {
    fn to_string(&self) -> String {
        let mut s = format!("{}", self.code.to_u32());
        if let Some(ref data) = self.data {
            s.push_str(" ");
            s.push_str(data);
        }
        s.push_str("\n");
        s
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::common::protocol::status::StatusCode;

    #[test]
    fn test_response_to_string() {
        let resp = Response {
            code: ResponseCode::Status(StatusCode::Ok),
            data: Some("Hello".to_string()),
        };
        assert_eq!(resp.to_string(), "200 Hello\n");
    }

    #[test]
    fn test_response_from_str() {
        let resp = Response::from_str("200 Hello").unwrap();
        assert_eq!(resp.code, ResponseCode::Status(StatusCode::Ok));
        assert_eq!(resp.data, Some("Hello".to_string()));
    }

    #[test]
    fn test_response_from_str_no_data() {
        let resp = Response::from_str("200").unwrap();
        assert_eq!(resp.code, ResponseCode::Status(StatusCode::Ok));
        assert_eq!(resp.data, None);
    }

    #[test]
    fn test_response_from_str_invalid() {
        assert!(Response::from_str("999").is_err());
        assert!(Response::from_str("abc").is_err());
    }
}
