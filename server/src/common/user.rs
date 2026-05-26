use crate::common::utils::serializing::{read_str, write_str};
use crate::common::utils::uuid_v4;
use std::fs::File;

#[derive(Clone)]
pub struct User {
    pub uuid: String,
    pub name: String,
}

impl User {
    pub fn new(name: String) -> Self {
        Self {
            uuid: uuid_v4(),
            name,
        }
    }

    pub fn from_string(s: &str) -> Option<Self> {
        let parts: Vec<&str> = s.splitn(2, ' ').collect();
        if parts.len() != 2 {
            return None;
        }
        Some(Self {
            uuid: parts[0].to_string(),
            name: parts[1].to_string(),
        })
    }

    pub fn to_string(&self) -> String {
        format!("{} {}", self.uuid, self.name)
    }

    pub fn write_to_file(&self, file: &mut File) {
        write_str(file, &self.to_string()).unwrap();
    }

    pub fn read_from_file(file: &mut File) -> User {
        let msg = read_str(file).unwrap();
        let user = User::from_string(&msg).expect("Error importing message from file");
        user
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_user_new() {
        let name = "John Doe".to_string();
        let user = User::new(name.clone());
        assert_eq!(user.name, name);
        assert_eq!(user.uuid.len(), 36);
    }

    #[test]
    fn test_user_to_string() {
        let user = User {
            uuid: "1234-5678-9012-3456".to_string(),
            name: "John Doe".to_string(),
        };
        assert_eq!(user.to_string(), "1234-5678-9012-3456 John Doe");
    }

    #[test]
    fn test_user_from_string() {
        let s = "1234-5678-9012-3456 John Doe";
        let user = User::from_string(s).unwrap();
        assert_eq!(user.uuid, "1234-5678-9012-3456");
        assert_eq!(user.name, "John Doe");
    }

    #[test]
    fn test_user_from_invalid_string() {
        assert!(User::from_string("invalid_format").is_none());
    }
}
