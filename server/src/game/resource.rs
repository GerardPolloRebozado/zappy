#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Resource {
    Food,
    Linemate,
    Deraumere,
    Sibur,
    Mendiane,
    Phiras,
    Thystame,
}

impl std::fmt::Display for Resource {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let name = match self {
            Resource::Food => "food",
            Resource::Linemate => "linemate",
            Resource::Deraumere => "deraumere",
            Resource::Sibur => "sibur",
            Resource::Mendiane => "mendiane",
            Resource::Phiras => "phiras",
            Resource::Thystame => "thystame",
        };
        write!(f, "{}", name)
    }
}
