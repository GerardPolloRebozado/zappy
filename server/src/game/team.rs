use crate::utils::serializing::{read_length, read_str, write_length, write_str};
use crate::utils::uuid_v4;
use std::fs::File;

pub struct Team {
    pub uuid: String,
    pub name: String,
    pub users_uuid: Vec<String>,
}

impl Team {
    pub fn new(name: String) -> Self {
        Team {
            uuid: uuid_v4(),
            name,
            users_uuid: Vec::new(),
        }
    }

    pub fn write_to_file(&self, file: &mut File) {
        write_str(file, self.uuid.as_str()).unwrap();
        write_str(file, self.name.as_str()).unwrap();

        write_length(file, self.users_uuid.len() as u64).unwrap();
        for uuid in &self.users_uuid {
            write_str(file, uuid).unwrap()
        }
    }

    pub fn read_from_file(file: &mut File) -> Team {
        let uuid = read_str(file).unwrap();
        let name = read_str(file).unwrap();
        let mut team = Team {
            uuid,
            name,
            users_uuid: Vec::new(),
        };

        let users_length = read_length(file).unwrap();
        for _ in 0..users_length {
            team.users_uuid.push(read_str(file).unwrap());
        }

        team
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::remove_file;

    #[test]
    fn test_team_new() {
        let name = "Test Team".to_string();
        let team = Team::new(name.clone());
        assert_eq!(team.name, name);
        assert!(!team.uuid.is_empty());
        assert!(team.users_uuid.is_empty());
    }

    #[test]
    fn test_team_file_io() {
        let mut team = Team::new("Team IO".to_string());
        team.users_uuid.push("user-1".to_string());

        let filename = "test_team.txt";
        {
            let mut file = File::create(filename).unwrap();
            team.write_to_file(&mut file);
        }

        {
            let mut file = File::open(filename).unwrap();
            let read_team = Team::read_from_file(&mut file);
            assert_eq!(read_team.uuid, team.uuid);
            assert_eq!(read_team.name, team.name);
            assert_eq!(read_team.users_uuid, team.users_uuid);
        }
        let _ = remove_file(filename);
    }
}
