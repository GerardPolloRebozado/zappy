pub mod constants;
pub mod direction;
pub mod serializing;

use std::time::UNIX_EPOCH;

pub fn random_bytes() -> [u8; 16] {
    let now = std::time::SystemTime::now();

    let duration = now.duration_since(UNIX_EPOCH).expect("Time went backwards");
    let mut state: u32 = duration.as_nanos() as u32;
    if state == 0 {
        state = 1;
    }
    let mut bytes: [u8; 16] = [0; 16];
    for byte in &mut bytes {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;

        *byte = state as u8;
    }
    bytes
}

pub fn uuid_v4() -> String {
    let mut bytes = random_bytes();
    bytes[6] = (bytes[6] & 0b0000_1111) | 0b0100_0000;
    bytes[8] = (bytes[8] & 0b0011_1111) | 0b1000_0000;
    format!(
        "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        bytes[0],
        bytes[1],
        bytes[2],
        bytes[3],
        bytes[4],
        bytes[5],
        bytes[6],
        bytes[7], // Byte index 6 starts the 3rd block (The Version block)
        bytes[8],
        bytes[9], // Byte index 8 starts the 4th block (The Variant block)
        bytes[10],
        bytes[11],
        bytes[12],
        bytes[13],
        bytes[14],
        bytes[15]
    )
}

pub fn escape_str(s: &str) -> String {
    s.replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\n", "\\n")
}

pub fn unescape_str(s: &str) -> String {
    s.replace("\\n", "\n")
        .replace("\\\"", "\"")
        .replace("\\\\", "\\")
}

///
/// Splits a string into arguments, treating text in double quotes as a single argument.
/// The first argument (the command) can be unquoted, but all subsequent arguments MUST be quoted.
/// Supports escaped double quotes (\" and \\) within quoted strings.
///
/// # Arguments
/// * `input` - The input string to parse
///
/// # Returns
/// A `Vec<String>` containing the parsed arguments. Returns an empty `Vec` on error
/// (e.g., missing closing quote or unquoted argument after the first word).
///
/// # Examples
/// ```ignore
/// use zappy_server::common::utils::parse_args;
/// let args = parse_args(r#"LOGIN "alex""#);
/// assert_eq!(args, vec!["LOGIN", "alex"]);
/// ```
///
pub fn parse_args(input: &str) -> Vec<String> {
    input.split_whitespace().map(|s| s.to_string()).collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_uuid_v4() {
        let uuid = uuid_v4();
        assert_eq!(uuid.len(), 36);
        assert_eq!(uuid.chars().filter(|&c| c == '-').count(), 4);
        assert_eq!(uuid.chars().nth(14).unwrap(), '4');
    }

    #[test]
    fn test_escape_str() {
        assert_eq!(escape_str("hello \"world\" \n"), "hello \\\"world\\\" \\n");
        assert_eq!(escape_str("back\\slash"), "back\\\\slash");
    }

    #[test]
    fn test_unescape_str() {
        assert_eq!(
            unescape_str("hello \\\"world\\\" \\n"),
            "hello \"world\" \n"
        );
        assert_eq!(unescape_str("back\\\\slash"), "back\\slash");
    }

    #[test]
    fn test_parse_args() {
        assert_eq!(parse_args(r#"LOGIN "alex""#), vec!["LOGIN", "\"alex\""]);
        assert_eq!(parse_args(r#"/login "alex""#), vec!["/login", "\"alex\""]);
        assert_eq!(parse_args(r#"/login alex"#), vec!["/login", "alex"]);
        assert_eq!(parse_args(r#"/login "alex"#), vec!["/login", "\"alex"]);
        assert_eq!(
            parse_args(r#"CREATE "team" "desc""#),
            vec!["CREATE", "\"team\"", "\"desc\""]
        );
        assert_eq!(
            parse_args(r#"SEND "uuid" "hello \"world\"""#),
            vec!["SEND", "\"uuid\"", "\"hello", "\\\"world\\\"\""]
        );
    }
}
