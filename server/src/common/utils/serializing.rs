use std::fs::File;
use std::io;
use std::io::{Read, Write};

pub fn write_length(file: &mut File, len: u64) -> io::Result<()>{
    file.write_all(&len.to_le_bytes())?;
    Ok(())
}

pub fn read_length(file: &mut File) -> io::Result<usize> {
    let mut len_bytes = [0u8; 8];
    file.read_exact(&mut len_bytes)?;
    Ok(u64::from_le_bytes(len_bytes) as usize)
}

pub fn write_str(file: &mut File, s: &str) -> io::Result<()> {
    let bytes = s.as_bytes();
    write_length(file, bytes.len() as u64)?;
    file.write_all(bytes)?;
    Ok(())
}

pub fn read_str(file: &mut File) -> io::Result<String> {
    let len = read_length(file)?;
    let mut buffer = vec![0u8; len];
    file.read_exact(&mut buffer)?;

    String::from_utf8(buffer).map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e))
}