pub mod constants;
pub mod date;
pub mod orientation;
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
/// use zappy_engine::utils::parse_args;
/// let args = parse_args(r#"LOGIN "alex""#);
/// assert_eq!(args, vec!["LOGIN", "alex"]);
/// ```
///
pub fn parse_args(input: &str) -> Vec<String> {
    input.split_whitespace().map(|s| s.to_string()).collect()
}

impl Default for Config {
    fn default() -> Self {
        Self {
            port: 8080,
            width: 100,
            height: 100,
            names: vec!["TeamA".to_string(), "TeamB".to_string()],
            clients_nb: 1,
            freq: 100,
            seed: None,
        }
    }
}

#[derive(PartialEq, Debug)]
pub struct Config {
    pub port: u16,
    pub width: u32,
    pub height: u32,
    pub names: Vec<String>,
    pub clients_nb: u32,
    pub freq: u32,
    pub seed: Option<String>,
}

///
/// Parse the command line argument with multiple options.
///
/// # Arguments
/// * `args` - The vector of string to parse
///
/// # Returns
/// A `Config` structure is return on Succes containing the parsed argument. On Error retrun an `Err(reason)` corresponding to the error.
/// (e.g., invalid value, missing arguments).
///
/// # Examples
/// ```ignore
/// // Equivalent of "./zappy_server -p 4242 -x 128 -y 96 -n alpha beta -c 4 -f 20"
/// let arguments: Vec<String> = vec![
///    "zappy_server".to_string(),
///    "-p".to_string(), "4242".to_string(),
///    "-x".to_string(), "128".to_string(),
///    "-y".to_string(), "96".to_string(),
///    "-n".to_string(), "alpha".to_string(), "beta".to_string(),
///    "-c".to_string(), "4".to_string(),
///    "-f".to_string(), "20".to_string(),
///    ];
///
/// // The Config Result
/// let config: Config = Config{
///     port: 4242,
///     width: 128,
///     height: 96,
///     names: vec!["alpha".to_string(),
///     "beta".to_string()],
///     clients_nb: 4,
///     freq: 20
/// };
///
/// assert_eq!(parse_server_args(&arguments).unwrap(), config);
/// ```
///
pub fn parse_server_args(args: &[String]) -> Result<Config, String> {
    let mut port: Option<u16> = None;
    let mut width: Option<u32> = None;
    let mut height: Option<u32> = None;
    let mut names: Option<Vec<String>> = None;
    let mut clients_nb: Option<u32> = None;
    let mut freq: Option<u32> = None;
    let mut seed: Option<String> = None;
    let mut i = 1;

    while i < args.len() {
        match args[i].as_str() {
            "-p" => {
                i += 1;
                let value = args.get(i).ok_or("Missing value for -p")?;
                port = Some(value.parse::<u16>().map_err(|_| "Invalid value for -p")?);
            }
            "-x" => {
                i += 1;
                let value = args.get(i).ok_or("Missing value for -x")?;
                width = Some(value.parse::<u32>().map_err(|_| "Invalid value for -x")?);
            }
            "-y" => {
                i += 1;
                let value = args.get(i).ok_or("Missing value for -y")?;
                height = Some(value.parse::<u32>().map_err(|_| "Invalid value for -y")?);
            }
            "-c" => {
                i += 1;
                let value = args.get(i).ok_or("Missing value for -c")?;
                clients_nb = Some(value.parse::<u32>().map_err(|_| "Invalid value for -c")?);
            }
            "-f" => {
                i += 1;
                let value = args.get(i).ok_or("Missing value for -f")?;
                freq = Some(value.parse::<u32>().map_err(|_| "Invalid value for -f")?);
            }
            "--seed" => {
                i += 1;
                let value = args.get(i).ok_or("Missing value for --seed")?;
                seed = Some(value.clone());
            }
            "-n" => {
                i += 1;
                let mut team_names = Vec::new();

                while i < args.len() && !args[i].starts_with('-') {
                    team_names.push(args[i].clone());
                    i += 1;
                }

                if team_names.is_empty() {
                    return Err("Missing team names after -n".into());
                }

                names = Some(team_names);
                continue;
            }
            unknown => {
                return Err(format!("Unknown option: {unknown}"));
            }
        }
        i += 1;
    }

    Ok(Config {
        port: port.ok_or("Missing -p")?,
        width: width.unwrap_or(100),
        height: height.unwrap_or(100),
        clients_nb: clients_nb.ok_or("Missing -c")?,
        names: names.unwrap_or_else(|| vec!["team".to_string()]),
        freq: freq.unwrap_or(100),
        seed,
    })
}

#[cfg(test)]
mod tests {
    use std::vec;

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

    #[test]
    fn test_parse_server_args() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-p".to_string(),
            "4242".to_string(),
            "-x".to_string(),
            "128".to_string(),
            "-y".to_string(),
            "96".to_string(),
            "-n".to_string(),
            "alpha".to_string(),
            "beta".to_string(),
            "-c".to_string(),
            "4".to_string(),
            "-f".to_string(),
            "20".to_string(),
        ];

        let config: Config = Config {
            port: 4242,
            width: 128,
            height: 96,
            names: vec!["alpha".to_string(), "beta".to_string()],
            clients_nb: 4,
            freq: 20,
            seed: None,
        };

        assert_eq!(parse_server_args(&arguments).unwrap(), config);
    }

    #[test]
    fn test_parse_server_args_default_args() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-p".to_string(),
            "4242".to_string(),
            "-c".to_string(),
            "4".to_string(),
        ];

        let config: Config = Config {
            port: 4242,
            width: 100,
            height: 100,
            names: vec!["team".to_string()],
            clients_nb: 4,
            freq: 100,
            seed: None,
        };

        assert_eq!(parse_server_args(&arguments).unwrap(), config);
    }

    #[test]
    fn test_parse_server_args_missing_args_port() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-c".to_string(),
            "4".to_string(),
        ];

        assert_eq!(parse_server_args(&arguments).unwrap_err(), "Missing -p");
    }

    #[test]
    fn test_parse_server_args_missing_args_client_number() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-p".to_string(),
            "4242".to_string(),
        ];

        assert_eq!(parse_server_args(&arguments).unwrap_err(), "Missing -c");
    }

    #[test]
    fn test_parse_server_args_missing_value_port() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-c".to_string(),
            "4".to_string(),
            "-p".to_string(),
        ];

        assert_eq!(
            parse_server_args(&arguments).unwrap_err(),
            "Missing value for -p"
        );
    }

    #[test]
    fn test_parse_server_args_missing_value_name() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-p".to_string(),
            "4242".to_string(),
            "-n".to_string(),
        ];

        assert_eq!(
            parse_server_args(&arguments).unwrap_err(),
            "Missing team names after -n"
        );
    }

    #[test]
    fn test_parse_server_args_invalid_value() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-p".to_string(),
            "not a number".to_string(),
        ];

        assert_eq!(
            parse_server_args(&arguments).unwrap_err(),
            "Invalid value for -p"
        );
    }

    #[test]
    fn test_parse_server_args_unknow_flag() {
        let arguments: Vec<String> = vec![
            "zappy_server".to_string(),
            "-p".to_string(),
            "4242".to_string(),
            "-a".to_string(),
        ];

        assert_eq!(
            parse_server_args(&arguments).unwrap_err(),
            "Unknown option: -a"
        );
    }
}
