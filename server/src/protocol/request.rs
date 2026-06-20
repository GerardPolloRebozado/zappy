use crate::protocol::Command;
use crate::utils::parse_args;
use std::str::FromStr;

#[derive(Debug, Clone, PartialEq)]
pub struct Request {
    pub command: Command,
}

impl std::fmt::Display for Request {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "{}", self.command)
    }
}

impl FromStr for Request {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let s = s.trim();
        if s.is_empty() {
            return Err(());
        }

        let args = parse_args(s);
        if args.is_empty() {
            return Err(());
        }

        let cmd_name = args[0].to_string();

        let command = match cmd_name.as_str() {
            "Forward" => Command::Forward,
            "Right" => Command::Right,
            "Left" => Command::Left,
            "Look" => Command::Look,
            "Inventory" => Command::Inventory,
            "Broadcast" if args.len() > 1 => Command::Broadcast(args[1..].join(" ")),
            "Connect_nbr" => Command::ConnectNbr,
            "Fork" => Command::Fork,
            "Eject" => Command::Eject,
            "Take" if args.len() > 1 => Command::Take(args[1].clone()),
            "Set" if args.len() > 1 => Command::Set(args[1].clone()),
            "Incantation" => Command::Incantation,

            "msz" => Command::Msz,
            "bct" if args.len() > 2 => {
                let x = args[1].parse().map_err(|_| ())?;
                let y = args[2].parse().map_err(|_| ())?;
                Command::Bct(x, y)
            }
            "mct" => Command::Mct,
            "tna" => Command::Tna,
            "ppo" if args.len() > 1 => Command::Ppo(args[1].clone()),
            "plv" if args.len() > 1 => Command::Plv(args[1].clone()),
            "pin" if args.len() > 1 => Command::Pin(args[1].clone()),
            "sgt" => Command::Sgt,
            "sst" if args.len() > 1 => {
                let t = args[1].parse().map_err(|_| ())?;
                Command::Sst(t)
            }

            "mev" if args.len() > 1 => Command::Mev(args[1].clone()),
            "gev" => Command::Gev,

            _ => Command::Unknown(cmd_name),
        };

        Ok(Request { command })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse_mev_with_event_name() {
        let req: Request = "mev meteor_shower".parse().unwrap();
        assert_eq!(req.command, Command::Mev("meteor_shower".to_string()));
    }

    #[test]
    fn parse_mev_without_name_is_unknown() {
        let req: Request = "mev".parse().unwrap();
        assert_eq!(req.command, Command::Unknown("mev".to_string()));
    }

    #[test]
    fn parse_gev() {
        let req: Request = "gev".parse().unwrap();
        assert_eq!(req.command, Command::Gev);
    }
}
