use crate::{
    ecs::map_size::MapSize,
    game::Resource::{Deraumere, Food, Linemate, Mendiane, Phiras, Sibur, Thystame},
    utils::constants::{
        DERAUMERE_DENSITY, FOOD_DENSITY, LINEMATE_DENSITY, MENDIANE_DENSITY, PHIRAS_DENSITY,
        SIBUR_DENSITY, THYSTAME_DENSITY,
    },
};

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

impl Resource {
    pub fn iter() -> impl Iterator<Item = Resource> {
        [Food, Linemate, Deraumere, Sibur, Mendiane, Phiras, Thystame]
            .iter()
            .copied()
    }

    pub fn get_density(&self) -> f32 {
        match self {
            Resource::Food => FOOD_DENSITY,
            Resource::Linemate => LINEMATE_DENSITY,
            Resource::Deraumere => DERAUMERE_DENSITY,
            Resource::Sibur => SIBUR_DENSITY,
            Resource::Mendiane => MENDIANE_DENSITY,
            Resource::Phiras => PHIRAS_DENSITY,
            Resource::Thystame => THYSTAME_DENSITY,
        }
    }

    /// returns the maximum quantity of this resource that can be present on the map at once, based on the map size and resource density
    pub fn get_resource_max_quantity(&self, map_size: MapSize) -> u64 {
        (map_size.height as f32 * map_size.width as f32 * self.get_density()).ceil() as u64
    }
    pub fn new_from_str(s: &str) -> Option<Resource> {
        match s {
            "food" => Some(Resource::Food),
            "linemate" => Some(Resource::Linemate),
            "deraumere" => Some(Resource::Deraumere),
            "sibur" => Some(Resource::Sibur),
            "mendiane" => Some(Resource::Mendiane),
            "phiras" => Some(Resource::Phiras),
            "thystame" => Some(Resource::Thystame),
            _ => None,
        }
    }
}
