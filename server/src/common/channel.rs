use crate::common::Thread;
use crate::common::utils::serializing::{read_length, read_str, write_length, write_str};
use crate::common::utils::uuid_v4;
use std::collections::HashMap;
use std::fs::File;

#[derive(Clone)]
pub struct Channel {
    pub uuid: String,
    pub name: String,
    pub description: String,
    pub threads: HashMap<String, Thread>,
}
impl Channel {
    pub fn new(name: &str) -> Self {
        Channel {
            uuid: uuid_v4(),
            name: String::from(name),
            description: String::new(),
            threads: HashMap::new(),
        }
    }

    pub fn write_to_file(&self, file: &mut File) {
        write_str(file, &self.uuid).unwrap();
        write_str(file, &self.name).unwrap();
        write_str(file, &self.description).unwrap();

        write_length(file, self.threads.len() as u64).unwrap();
        for thread in &self.threads {
            thread.1.write_to_file(file);
        }
    }

    pub fn read_from_file(file: &mut File) -> Channel {
        let uuid = read_str(file).unwrap();
        let name = read_str(file).unwrap();
        let description = read_str(file).unwrap();
        let mut channel = Channel {
            uuid,
            name,
            description,
            threads: HashMap::new(),
        };

        let messages_length = read_length(file).unwrap();
        for _ in 0..messages_length {
            let thread = Thread::read_from_file(file);
            channel.threads.insert(thread.uuid.clone(), thread);
        }
        channel
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::remove_file;

    #[test]
    fn test_channel_new() {
        let name = "Test Channel";
        let channel = Channel::new(name);
        assert_eq!(channel.name, name);
        assert!(channel.uuid.len() > 0);
        assert!(channel.threads.is_empty());
    }

    #[test]
    fn test_channel_file_io() {
        let mut channel = Channel::new("Channel IO");
        channel.description = "Channel Desc".to_string();

        let filename = "test_channel.txt";
        {
            let mut file = File::create(filename).unwrap();
            channel.write_to_file(&mut file);
        }

        {
            let mut file = File::open(filename).unwrap();
            let read_channel = Channel::read_from_file(&mut file);
            assert_eq!(read_channel.uuid, channel.uuid);
            assert_eq!(read_channel.name, channel.name);
            assert_eq!(read_channel.description, channel.description);
        }
        let _ = remove_file(filename);
    }
}
