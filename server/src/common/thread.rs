use crate::common::Message;
use crate::common::date::Date;
use crate::common::utils::serializing::{read_length, read_str, write_length, write_str};
use crate::common::utils::uuid_v4;
use std::fs::File;

#[derive(Clone)]
pub struct Thread {
    pub uuid: String,
    pub title: String,
    pub messages: Vec<Message>,
    pub creation_date: Date,
}

impl Thread {
    pub fn new(title: String) -> Thread {
        Thread {
            uuid: uuid_v4(),
            title,
            messages: Vec::new(),
            creation_date: Date::now(),
        }
    }
    pub fn write_to_file(&self, file: &mut File) {
        write_str(file, self.uuid.as_str()).unwrap();
        write_str(file, self.title.as_str()).unwrap();
        write_length(file, self.messages.len() as u64).unwrap();
        for message in self.messages.iter() {
            message.write_to_file(file);
        }
        write_str(file, self.creation_date.to_string().as_str()).unwrap();
    }

    pub fn read_from_file(file: &mut File) -> Thread {
        let uuid = read_str(file).unwrap();
        let title = read_str(file).unwrap();
        let mut thread = Thread::new(title);
        thread.uuid = uuid;

        let messages_len = read_length(file).unwrap();
        for _ in 0..messages_len {
            thread.messages.push(Message::read_from_file(file))
        }

        let creation_date = read_str(file).unwrap();
        thread.creation_date = Date::from_string(creation_date.as_str()).unwrap();
        thread
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::remove_file;

    #[test]
    fn test_thread_new() {
        let title = "Test Thread".to_string();
        let thread = Thread::new(title.clone());
        assert_eq!(thread.title, title);
        assert!(thread.uuid.len() > 0);
        assert!(thread.messages.is_empty());
    }

    #[test]
    fn test_thread_file_io() {
        let mut thread = Thread::new("Thread IO".to_string());

        let filename = "test_thread.txt";
        {
            let mut file = File::create(filename).unwrap();
            thread.write_to_file(&mut file);
        }

        {
            let mut file = File::open(filename).unwrap();
            let read_thread = Thread::read_from_file(&mut file);
            assert_eq!(read_thread.uuid, thread.uuid);
            assert_eq!(read_thread.title, thread.title);
            assert_eq!(
                read_thread.creation_date.to_string(),
                thread.creation_date.to_string()
            );
        }
        let _ = remove_file(filename);
    }
}
