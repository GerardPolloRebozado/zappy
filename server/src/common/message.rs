use crate::common::date::Date;
use crate::common::utils::serializing::{read_str, write_str};
use crate::common::utils::uuid_v4;
use std::fs::File;

#[derive(Clone)]
pub enum Location {
    THREAD,
    USER,
}

impl Location {
    pub fn to_string(&self) -> String {
        match self {
            Location::THREAD => "THREAD".to_string(),
            Location::USER => "USER".to_string(),
        }
    }

    pub fn from_string(s: &str) -> Option<Location> {
        match s {
            "THREAD" => Some(Location::THREAD),
            "USER" => Some(Location::USER),
            _ => None,
        }
    }
}

#[derive(Clone)]
pub struct Message {
    pub uuid: String,
    pub message: String,
    pub sender_uuid: String,
    pub date: Date,
    pub location_type: Location,
    pub location_uuid: String,
}

impl Message {
    pub fn new(
        sender_uuid: String,
        location_type: Location,
        location_uuid: String,
        message: String,
    ) -> Message {
        Message {
            uuid: uuid_v4(),
            message,
            sender_uuid,
            date: Date::now(),
            location_type,
            location_uuid,
        }
    }

    pub fn write_to_file(&self, file: &mut File) {
        write_str(file, self.to_string().as_str()).unwrap();
    }

    pub fn read_from_file(file: &mut File) -> Message {
        let msg = read_str(file).unwrap();
        Message::from_string(&msg).expect("Error importing message from file")
    }

    pub fn to_string(&self) -> String {
        format!(
            "{}|{}|{}|{}|{}|{}",
            self.uuid,
            self.sender_uuid,
            self.date.to_string(),
            self.location_type.to_string(),
            self.location_uuid,
            self.message
        )
    }

    pub fn from_string(message: &str) -> Option<Message> {
        let parts: Vec<&str> = message.splitn(6, '|').collect();

        if parts.len() != 6 {
            return None;
        }

        let uuid = parts[0].to_string();
        let sender = parts[1].to_string();
        let date = Date::from_string(parts[2])?;
        let location_type = Location::from_string(parts[3])?;
        let location_uuid = parts[4].to_string();
        let msg = parts[5].to_string();

        Some(Message {
            uuid,
            message: msg,
            sender_uuid: sender,
            date,
            location_type,
            location_uuid,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_location_to_string() {
        assert_eq!(Location::THREAD.to_string(), "THREAD");
        assert_eq!(Location::USER.to_string(), "USER");
    }

    #[test]
    fn test_location_from_string() {
        assert!(matches!(
            Location::from_string("THREAD"),
            Some(Location::THREAD)
        ));
        assert!(matches!(
            Location::from_string("USER"),
            Some(Location::USER)
        ));
        assert!(Location::from_string("INVALID").is_none());
    }

    #[test]
    fn test_message_to_string() {
        let date = Date {
            day: 1,
            month: 1,
            year: 2024,
            hour: 12,
            minute: 0,
        };
        let msg = Message {
            uuid: "msg-uuid".to_string(),
            message: "hello world".to_string(),
            sender_uuid: "sender-uuid".to_string(),
            date,
            location_type: Location::USER,
            location_uuid: "receiver-uuid".to_string(),
        };
        assert_eq!(
            msg.to_string(),
            "msg-uuid|sender-uuid|01/01/2024 12:00|USER|receiver-uuid|hello world"
        );
    }

    #[test]
    fn test_message_from_string() {
        let s = "msg-uuid|sender-uuid|01/01/2024 12:00|USER|receiver-uuid|hello world";
        let msg = Message::from_string(s).unwrap();
        assert_eq!(msg.uuid, "msg-uuid");
        assert_eq!(msg.sender_uuid, "sender-uuid");
        assert_eq!(msg.date.to_string(), "01/01/2024 12:00");
        assert!(matches!(msg.location_type, Location::USER));
        assert_eq!(msg.location_uuid, "receiver-uuid");
        assert_eq!(msg.message, "hello world");
    }

    #[test]
    fn test_message_from_invalid_string() {
        assert!(Message::from_string("invalid").is_none());
        assert!(Message::from_string("a|b|c|d|e").is_none());
    }
}
