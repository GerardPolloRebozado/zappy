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

        let args = parse_args(s);
        if args.is_empty() {
            return Err(());
        }

        let cmd_name = args[0].to_string();

        let command = match cmd_name.as_str() {
            // AI Commands
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

            // GUI Commands
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

            // For the handshake, any non-command string in the first message is handled by the server state
            _ => Command::Unknown(cmd_name),
        };

        Ok(Request { command })
    }
}
