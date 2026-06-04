use crate::ecs::components::life::Life;
use crate::ecs::components::position::Position;
use crate::ecs::storage::{Entity, World};
use crate::utils::orientation::RelativeOrientation;

/// A read-only snapshot of an inhabitant's state on the game map.
///
/// This struct provides a view of an entity's components.
/// Modifying an `Inhabitant` instance does not affect the ECS storage.
/// Fields are private to prevent accidental local modifications that
/// wouldn't be reflected in the "real" data.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Inhabitant {
    id: u32,
    x: u32,
    y: u32,
    /// Facing direction on the map (`1` = north, `2` = east, `3` = south, `4` = west).
    orientation: RelativeOrientation,
    life: Life,
}

impl Default for Inhabitant {
    fn default() -> Self {
        Self {
            id: 0,
            x: 0,
            y: 0,
            orientation: RelativeOrientation::Forward,
            life: Life::new(100),
        }
    }
}

impl Inhabitant {
    /// Creates an inhabitant snapshot at the given position and orientation
    pub fn new(id: u32, x: u32, y: u32, orientation: RelativeOrientation, life: Life) -> Self {
        Self {
            id,
            x,
            y,
            orientation,
            life,
        }
    }

    // Creates an inhabitant with a specific id
    pub fn with_id(mut self, id: u32) -> Self {
        self.id = id;
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

    /// Fetches an inhabitant's state from the ECS world as a read-only snapshot.
    ///
    /// Returns `None` if the entity is missing required components.
    pub fn get(entity: Entity, world: &World) -> Option<Self> {
        let pos = world.get_component::<Position>(entity)?;
        let ori = world.get_component::<RelativeOrientation>(entity)?;
        let life = world.get_component::<Life>(entity)?;

        Some(Self {
            id: entity.id(),
            x: pos.x,
            y: pos.y,
            orientation: *ori,
            life: *life,
        })
    }

    /// Returns the player's unique identifier.
    pub fn id(&self) -> u32 {
        self.id
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
}
