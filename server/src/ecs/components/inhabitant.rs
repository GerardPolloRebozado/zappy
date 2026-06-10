use crate::ecs::components::inventory::Inventory;
use crate::ecs::components::level::Level;
use crate::ecs::components::life::Life;
use crate::ecs::components::position::Position;
use crate::ecs::components::team::Team;
use crate::ecs::storage::{Entity, World};
use crate::utils::orientation::RelativeOrientation;
use crate::utils::serializing::{read_str, write_str};
use crate::utils::uuid_v4;
use std::fmt;
use std::fs::File;

/// A read-only snapshot of an inhabitant's state on the game map.
///
/// This struct provides a view of an entity's components.
/// Modifying an `Inhabitant` instance does not affect the ECS storage.
/// Fields are private to prevent accidental local modifications that
/// wouldn't be reflected in the "real" data.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Inhabitant {
    id: u32,
    uuid: String,
    name: String,
    x: u32,
    y: u32,
    /// Facing direction on the map (`1` = north, `2` = east, `3` = south, `4` = west).
    orientation: RelativeOrientation,
    life: Life,
    level: Level,
    inventory: Inventory,
    team: Team,
}

impl Default for Inhabitant {
    fn default() -> Self {
        Self {
            id: 0,
            uuid: uuid_v4(),
            name: "default".to_string(),
            x: 0,
            y: 0,
            orientation: RelativeOrientation::Forward,
            life: Life::new(100),
            level: Level::default(),
            inventory: Inventory::default(),
            team: Team::default(),
        }
    }
}

impl Inhabitant {
    /// Creates an inhabitant snapshot at the given position and orientation
    #[allow(clippy::too_many_arguments)]
    pub fn new(
        id: u32,
        uuid: String,
        name: String,
        x: u32,
        y: u32,
        orientation: RelativeOrientation,
        life: Life,
        level: Level,
        inventory: Inventory,
        team: Team,
    ) -> Self {
        Self {
            id,
            uuid,
            name,
            x,
            y,
            orientation,
            life,
            level,
            inventory,
            team,
        }
    }

    pub fn from_string(s: &str) -> Option<Self> {
        let parts: Vec<&str> = s.split(' ').collect();
        if parts.len() < 2 {
            return None;
        }
        let uuid = parts[0].to_string();
        let name = parts[1].to_string();

        Some(Self {
            uuid,
            name,
            ..Default::default()
        })
    }

    // Creates an inhabitant with a specific id
    pub fn with_id(mut self, id: u32) -> Self {
        self.id = id;
        self
    }

    // Creates an inhabitant with a specific uuid
    pub fn with_uuid(mut self, uuid: String) -> Self {
        self.uuid = uuid;
        self
    }

    // Creates an inhabitant with a specific name
    pub fn with_name(mut self, name: String) -> Self {
        self.name = name;
        self
    }

    // Creates an inhabitant with a specific position
    pub fn with_pos(mut self, x: u32, y: u32) -> Self {
        self.x = x;
        self.y = y;
        self
    }

    // Creates an inhabitant with a specific orientation
    pub fn with_orientation(mut self, orientation: RelativeOrientation) -> Self {
        self.orientation = orientation;
        self
    }

    // Creates an inhabitant with a specific life
    pub fn with_life(mut self, life: Life) -> Self {
        self.life = life;
        self
    }

    // Creates an inhabitant with a specific level
    pub fn with_level(mut self, level: Level) -> Self {
        self.level = level;
        self
    }

    // Creates an inhabitant with a specific inventory
    pub fn with_inventory(mut self, inventory: Inventory) -> Self {
        self.inventory = inventory;
        self
    }

    /// Fetches an inhabitant's state from the ECS world as a read-only snapshot.
    ///
    /// Returns `None` if the entity is missing required components.
    pub fn get(entity: Entity, world: &World) -> Option<Self> {
        let pos = world.get_component::<Position>(entity)?;
        let ori = world.get_component::<RelativeOrientation>(entity)?;
        let life = world.get_component::<Life>(entity)?;
        let team = world.get_component::<Team>(entity)?;
        let level = world.get_component::<Level>(entity)?;
        let inventory = world.get_component::<Inventory>(entity)?;

        // Default UUID and name since they aren't ECS components yet
        let uuid = uuid_v4();
        let name = format!("Player{}", entity.id());

        Some(Self {
            id: entity.id(),
            uuid,
            name,
            x: pos.x,
            y: pos.y,
            orientation: *ori,
            life: *life,
            level: *level,
            inventory: inventory.clone(),
            team: team.clone(),
        })
    }

    pub fn write_to_file(&self, file: &mut File) {
        write_str(file, &self.to_string()).unwrap();
    }

    pub fn read_from_file(file: &mut File) -> Self {
        let msg = read_str(file).unwrap();
        Self::from_string(&msg).expect("Error importing message from file")
    }

    /// Returns the player's unique identifier.
    pub fn id(&self) -> u32 {
        self.id
    }

    /// Returns the player's UUID.
    pub fn uuid(&self) -> &str {
        &self.uuid
    }

    /// Returns the player's name.
    pub fn name(&self) -> &str {
        &self.name
    }

    /// Returns the player's X coordinate.
    pub fn x(&self) -> u32 {
        self.x
    }

    /// Returns the player's Y coordinate.
    pub fn y(&self) -> u32 {
        self.y
    }

    /// Returns the player's facing direction.
    pub fn orientation(&self) -> RelativeOrientation {
        self.orientation
    }

    /// Returns the player's life.
    pub fn life(&self) -> Life {
        self.life
    }

    /// Returns the player's level.
    pub fn level(&self) -> Level {
        self.level
    }

    /// Returns the player's inventory.
    pub fn inventory(&self) -> &Inventory {
        &self.inventory
    }

    pub fn team(&self) -> &Team {
        &self.team
    }
}

impl fmt::Display for Inhabitant {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {}", self.uuid, self.name)
    }
}
