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
}

impl Inhabitant {
    /// Creates an inhabitant snapshot at the given position and orientation.
    pub fn new(id: u32, x: u32, y: u32, orientation: RelativeOrientation) -> Self {
        Self {
            id,
            x,
            y,
            orientation,
        }
    }

    /// Fetches an inhabitant's state from the ECS world as a read-only snapshot.
    ///
    /// Returns `None` if the entity is missing required components.
    pub fn get(entity: Entity, world: &World) -> Option<Self> {
        let pos = world.get_component::<Position>(entity)?;
        let ori = world.get_component::<RelativeOrientation>(entity)?;

        Some(Self {
            id: entity.id(),
            x: pos.x,
            y: pos.y,
            orientation: *ori,
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
