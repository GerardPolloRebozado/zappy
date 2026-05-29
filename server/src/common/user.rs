use crate::common::utils::serializing::{read_str, write_str};
use crate::common::utils::uuid_v4;
use std::fmt;
use std::fs::File;

use crate::common::Resource;
use std::collections::HashMap;

#[derive(Clone)]
pub struct User {
    pub uuid: String,
    pub name: String,
    pub level: u8,
    pub life_units: f32,
    pub inventory: HashMap<Resource, u32>,
}

impl User {
    pub fn new(name: String) -> Self {
        let mut inventory = HashMap::new();
        inventory.insert(Resource::Food, 10);

        Self {
            uuid: uuid_v4(),
            name,
            level: 1,
            life_units: 10.0,
            inventory,
        }
    }
    pub fn from_string(s: &str) -> Option<Self> {
        let parts: Vec<&str> = s.split(' ').collect();
        if parts.len() < 2 {
            return None;
        }
        let uuid = parts[0].to_string();
        let name = parts[1].to_string();

        let mut inventory = HashMap::new();
        inventory.insert(Resource::Food, 10);

        Some(Self {
            uuid,
            name,
            level: 1,
            life_units: 10.0,
            inventory,
        })
    }

    pub fn write_to_file(&self, file: &mut File) {
        write_str(file, &self.to_string()).unwrap();
    }

    pub fn read_from_file(file: &mut File) -> User {
        let msg = read_str(file).unwrap();
        User::from_string(&msg).expect("Error importing message from file")
    }
}

impl fmt::Display for User {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {}", self.uuid, self.name)
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
        let mut user = User::new("John Doe".to_string());
        user.uuid = "1234-5678-9012-3456".to_string();
        assert_eq!(user.to_string(), "1234-5678-9012-3456 John Doe");
    }

    #[test]
    fn test_user_from_invalid_string() {
        assert!(User::from_string("invalid_format").is_none());
    }
}
