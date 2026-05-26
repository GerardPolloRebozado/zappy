#[derive(Debug, Clone, PartialEq)]
pub enum Command {
    Login(String),
    Logout,
    Users,
    User(String),
    Unknown(String),
}

fn escape_string(s: &str) -> String {
    s.replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\n", "\\n")
}

impl std::fmt::Display for Command {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Command::Login(name) => write!(f, "LOGIN \"{}\"", escape_string(name)),
            Command::Logout => write!(f, "LOGOUT"),
            Command::Users => write!(f, "USERS"),
            Command::User(uuid) => write!(f, "USER \"{}\"", escape_string(uuid)),
            Command::Unknown(cmd) => write!(f, "UNKNOWN \"{}\"", escape_string(cmd)),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_command_display_login() {
        let cmd = Command::Login("alex".to_string());
        assert_eq!(cmd.to_string(), "LOGIN \"alex\"");
    }

    #[test]
    fn test_command_display_logout() {
        let cmd = Command::Logout;
        assert_eq!(cmd.to_string(), "LOGOUT");
    }

    #[test]
    fn test_escape_string() {
        assert_eq!(escape_string("hello \"world\""), "hello \\\"world\\\"");
        assert_eq!(escape_string("back\\slash"), "back\\\\slash");
    }
}
