use crate::common::protocol::command::Command;
use crate::common::utils::parse_args;
use std::str::FromStr;

#[derive(Debug, Clone, PartialEq)]
pub struct Request {
    pub command: Command,
}

impl ToString for Request {
    fn to_string(&self) -> String {
        format!("{}\n", self.command)
    }
}

impl FromStr for Request {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let s = s.trim();
        if s.is_empty() {
            return Err(());
        }

        let (cmd_name, args_str) = match s.find(' ') {
            Some(idx) => (&s[..idx], &s[idx..]),
            None => (s, ""),
        };

        let args = parse_args(&format!("CMD{}", args_str));
        if !args_str.trim().is_empty() && args.is_empty() {
            return Err(());
        }

        let command = match cmd_name.to_uppercase().as_str() {
            "LOGIN" if args.len() > 1 => Command::Login(args[1].clone()),
            "LOGOUT" => Command::Logout,
            "USERS" => Command::Users,
            "USER" if args.len() > 1 => Command::User(args[1].clone()),
            _ => Command::Unknown(cmd_name.to_string()),
        };

        Ok(Request { command })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_request_from_str_login() {
        let req = Request::from_str("LOGIN \"alex\"").unwrap();
        assert_eq!(req.command, Command::Login("alex".to_string()));
    }

    #[test]
    fn test_request_from_str_logout() {
        let req = Request::from_str("LOGOUT").unwrap();
        assert_eq!(req.command, Command::Logout);
    }
}
