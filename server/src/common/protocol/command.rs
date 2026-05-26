#[derive(Debug, Clone, PartialEq)]
pub enum Command {
    // AI Commands
    Forward,
    Right,
    Left,
    Look,
    Inventory,
    Broadcast(String),
    ConnectNbr,
    Fork,
    Eject,
    Take(String),
    Set(String),
    Incantation,

    // GUI Commands
    Msz,
    Bct(u32, u32),
    Mct,
    Tna,
    Ppo(String),
    Plv(String),
    Pin(String),
    Sgt,
    Sst(u32),

    // Internal / Skeleton leftovers
    Login(String), // This will be used for the Team Name handshake
    Unknown(String),
}

impl std::fmt::Display for Command {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Command::Forward => write!(f, "Forward"),
            Command::Right => write!(f, "Right"),
            Command::Left => write!(f, "Left"),
            Command::Look => write!(f, "Look"),
            Command::Inventory => write!(f, "Inventory"),
            Command::Broadcast(text) => write!(f, "Broadcast {}", text),
            Command::ConnectNbr => write!(f, "Connect_nbr"),
            Command::Fork => write!(f, "Fork"),
            Command::Eject => write!(f, "Eject"),
            Command::Take(obj) => write!(f, "Take {}", obj),
            Command::Set(obj) => write!(f, "Set {}", obj),
            Command::Incantation => write!(f, "Incantation"),

            Command::Msz => write!(f, "msz"),
            Command::Bct(x, y) => write!(f, "bct {} {}", x, y),
            Command::Mct => write!(f, "mct"),
            Command::Tna => write!(f, "tna"),
            Command::Ppo(id) => write!(f, "ppo {}", id),
            Command::Plv(id) => write!(f, "plv {}", id),
            Command::Pin(id) => write!(f, "pin {}", id),
            Command::Sgt => write!(f, "sgt"),
            Command::Sst(t) => write!(f, "sst {}", t),

            Command::Login(name) => write!(f, "{}", name),
            Command::Unknown(cmd) => write!(f, "{}", cmd),
        }
    }
}
