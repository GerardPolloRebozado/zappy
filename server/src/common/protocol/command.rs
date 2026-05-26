#[derive(Debug, Clone, PartialEq)]
pub enum Command {
    Login(String),
    Logout,
    Users,
    User(String),
    Send(String, String),
    Messages(String),
    Subscribe(String),
    Subscribed(Option<String>),
    Unsubscribe(String),
    Use(Option<String>, Option<String>, Option<String>),
    Create(Vec<String>),
    List,
    Info,
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
            Command::Send(uuid, msg) => {
                write!(
                    f,
                    "SEND \"{}\" \"{}\"",
                    escape_string(uuid),
                    escape_string(msg)
                )
            }
            Command::Messages(uuid) => write!(f, "MESSAGES \"{}\"", escape_string(uuid)),
            Command::Subscribe(uuid) => write!(f, "SUBSCRIBE \"{}\"", escape_string(uuid)),
            Command::Subscribed(uuid) => match uuid {
                Some(id) => write!(f, "SUBSCRIBED \"{}\"", escape_string(id)),
                None => write!(f, "SUBSCRIBED"),
            },
            Command::Unsubscribe(uuid) => write!(f, "UNSUBSCRIBE \"{}\"", escape_string(uuid)),
            Command::Use(t, c, th) => {
                let mut s = "USE".to_string();
                if let Some(id) = t {
                    s.push_str(&format!(" \"{}\"", escape_string(id)));
                }
                if let Some(id) = c {
                    s.push_str(&format!(" \"{}\"", escape_string(id)));
                }
                if let Some(id) = th {
                    s.push_str(&format!(" \"{}\"", escape_string(id)));
                }
                write!(f, "{}", s)
            }
            Command::Create(args) => {
                let mut s = "CREATE".to_string();
                for arg in args {
                    s.push_str(&format!(" \"{}\"", escape_string(arg)));
                }
                write!(f, "{}", s)
            }
            Command::List => write!(f, "LIST"),
            Command::Info => write!(f, "INFO"),
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
    fn test_command_display_send() {
        let cmd = Command::Send("uuid".to_string(), "hello world".to_string());
        assert_eq!(cmd.to_string(), "SEND \"uuid\" \"hello world\"");
    }

    #[test]
    fn test_command_display_use() {
        let cmd = Command::Use(Some("t1".to_string()), None, None);
        assert_eq!(cmd.to_string(), "USE \"t1\"");

        let cmd2 = Command::Use(Some("t1".to_string()), Some("c1".to_string()), None);
        assert_eq!(cmd2.to_string(), "USE \"t1\" \"c1\"");
    }

    #[test]
    fn test_command_display_create() {
        let cmd = Command::Create(vec!["team".to_string(), "desc".to_string()]);
        assert_eq!(cmd.to_string(), "CREATE \"team\" \"desc\"");
    }

    #[test]
    fn test_escape_string() {
        assert_eq!(escape_string("hello \"world\""), "hello \\\"world\\\"");
        assert_eq!(escape_string("back\\slash"), "back\\\\slash");
    }
}
