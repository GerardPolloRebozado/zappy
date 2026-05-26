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
            "SEND" if args.len() > 2 => Command::Send(args[1].clone(), args[2].clone()),
            "MESSAGES" if args.len() > 1 => Command::Messages(args[1].clone()),
            "SUBSCRIBE" if args.len() > 1 => Command::Subscribe(args[1].clone()),
            "SUBSCRIBED" => Command::Subscribed(args.get(1).cloned()),
            "UNSUBSCRIBE" if args.len() > 1 => Command::Unsubscribe(args[1].clone()),
            "USE" => Command::Use(
                args.get(1).cloned(),
                args.get(2).cloned(),
                args.get(3).cloned(),
            ),
            "CREATE" => Command::Create(if args.len() > 1 {
                args[1..].to_vec()
            } else {
                vec![]
            }),
            "LIST" => Command::List,
            "INFO" => Command::Info,
            _ => return Err(()),
        };

        Ok(Request { command })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::common::protocol::command::Command;

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

    #[test]
    fn test_request_from_str_send() {
        let req = Request::from_str("SEND \"uuid\" \"hello\"").unwrap();
        assert_eq!(
            req.command,
            Command::Send("uuid".to_string(), "hello".to_string())
        );
    }

    #[test]
    fn test_request_from_str_invalid() {
        assert!(Request::from_str("INVALID").is_err());
        assert!(Request::from_str("LOGIN").is_err()); // Missing arg
    }
}
