/// Active celestial anomaly affecting the whole world.
///
/// Tile-level modifiers from map.md are handled elsewhere:
/// - biomes → `TerrainType` on each tile
/// - landmarks → tile components (Monolith, EtherFissure)
/// - resource pulses → `resource_spawn_system` every 20 ticks
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub enum MapEvent {
    #[default]
    None,

    /// Rain of stellar fire that leaves tiles choked with random gems.
    /// Spawns `(W * H * 0.05)` random stones every 2 turns. Duration: 50 turns.
    MeteorShower {
        remaining_ticks: u32,
        ticks_until_next_spawn: u32,
    },

    /// Blinding burst of clarity: global max vision, food consumption at 2x speed.
    /// Duration: 20 turns.
    SolarFlare { remaining_ticks: u32 },

    /// Vortex that drags inhabitants within `R = min(W, H) / 4` one tile closer to its center
    /// every 5 turns. Duration: 30 turns.
    GravityWell {
        remaining_ticks: u32,
        ticks_until_next_pull: u32,
        center_x: u32,
        center_y: u32,
    },

    /// Ghostly whisper that reduces all active incantations' remaining time by 25%.
    /// Instant, one-shot effect applied when the event starts.
    PsionicEcho,
}

impl MapEvent {
    pub fn is_active(&self) -> bool {
        !matches!(self, MapEvent::None)
    }

    pub fn new_meteor_shower() -> Self {
        Self::MeteorShower {
            remaining_ticks: 50,
            ticks_until_next_spawn: 2,
        }
    }

    pub fn new_solar_flare() -> Self {
        Self::SolarFlare {
            remaining_ticks: 20,
        }
    }

    pub fn new_gravity_well(center_x: u32, center_y: u32) -> Self {
        Self::GravityWell {
            remaining_ticks: 30,
            ticks_until_next_pull: 5,
            center_x,
            center_y,
        }
    }
}
