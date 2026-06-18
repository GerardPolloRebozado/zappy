use rand::{RngExt, rng};

/// Interval in time units between random event trigger checks.
pub const EVENT_TRIGGER_INTERVAL: u64 = 50;
/// Probability of triggering a random event each check (0.0 to 1.0).
pub const EVENT_TRIGGER_CHANCE: f64 = 0.35;

/// Passive world-wide modifiers derived from the active [`MapEvent`].
///
/// Commands read these to adjust their behavior without knowing which
/// event is active. All fields default to neutral values (multipliers = 1.0,
/// overrides = `None`).
#[derive(Debug, Clone, Copy)]
pub struct WorldModifiers {
    /// Multiplier applied to food consumption rate (1.0 = normal, 2.0 = double).
    pub food_consumption_rate: f32,
    /// When `Some`, overrides the vision range for all inhabitants.
    pub vision_override: Option<u32>,
}

impl Default for WorldModifiers {
    fn default() -> Self {
        Self {
            food_consumption_rate: 1.0,
            vision_override: None,
        }
    }
}

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

    /// Returns the passive [`WorldModifiers`] implied by this event.
    pub fn modifiers(&self) -> WorldModifiers {
        match self {
            MapEvent::SolarFlare { .. } => WorldModifiers {
                food_consumption_rate: 2.0,
                vision_override: Some(u32::MAX),
            },
            _ => WorldModifiers::default(),
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

/// Rolls a random chance and, on success, returns a randomly selected [`MapEvent`].
///
/// The probability of triggering is [`EVENT_TRIGGER_CHANCE`]. When triggered,
/// one of the four event types is chosen uniformly at random. `GravityWell`
/// picks a random center position within the map bounds.
pub fn try_trigger_random_event(map_width: u32, map_height: u32) -> Option<MapEvent> {
    let mut rng = rng();

    if !rng.random_bool(EVENT_TRIGGER_CHANCE) {
        return None;
    }

    let variant = rng.random_range(0u32..4);
    let event = match variant {
        0 => MapEvent::new_meteor_shower(),
        1 => MapEvent::new_solar_flare(),
        2 => {
            let cx = rng.random_range(0..map_width);
            let cy = rng.random_range(0..map_height);
            MapEvent::new_gravity_well(cx, cy)
        }
        _ => MapEvent::PsionicEcho,
    };

    Some(event)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn world_modifiers_default_is_neutral() {
        let m = WorldModifiers::default();
        assert!((m.food_consumption_rate - 1.0).abs() < f32::EPSILON);
        assert_eq!(m.vision_override, None);
    }

    #[test]
    fn solar_flare_modifiers() {
        let m = MapEvent::new_solar_flare().modifiers();
        assert!((m.food_consumption_rate - 2.0).abs() < f32::EPSILON);
        assert_eq!(m.vision_override, Some(u32::MAX));
    }

    #[test]
    fn none_modifiers_are_neutral() {
        let m = MapEvent::None.modifiers();
        assert!((m.food_consumption_rate - 1.0).abs() < f32::EPSILON);
        assert_eq!(m.vision_override, None);
    }

    #[test]
    fn meteor_shower_modifiers_are_neutral() {
        let m = MapEvent::new_meteor_shower().modifiers();
        assert!((m.food_consumption_rate - 1.0).abs() < f32::EPSILON);
        assert_eq!(m.vision_override, None);
    }

    #[test]
    fn gravity_well_modifiers_are_neutral() {
        let m = MapEvent::new_gravity_well(5, 5).modifiers();
        assert!((m.food_consumption_rate - 1.0).abs() < f32::EPSILON);
        assert_eq!(m.vision_override, None);
    }

    #[test]
    fn psionic_echo_modifiers_are_neutral() {
        let m = MapEvent::PsionicEcho.modifiers();
        assert!((m.food_consumption_rate - 1.0).abs() < f32::EPSILON);
        assert_eq!(m.vision_override, None);
    }

    #[test]
    fn try_trigger_returns_valid_event_or_none() {
        for _ in 0..200 {
            if let Some(event) = try_trigger_random_event(10, 10) {
                match event {
                    MapEvent::None => panic!("should never return MapEvent::None"),
                    MapEvent::MeteorShower {
                        remaining_ticks, ..
                    } => {
                        assert_eq!(remaining_ticks, 50);
                    }
                    MapEvent::SolarFlare { remaining_ticks } => {
                        assert_eq!(remaining_ticks, 20);
                    }
                    MapEvent::GravityWell {
                        center_x,
                        center_y,
                        remaining_ticks,
                        ..
                    } => {
                        assert!(center_x < 10);
                        assert!(center_y < 10);
                        assert_eq!(remaining_ticks, 30);
                    }
                    MapEvent::PsionicEcho => {}
                }
            }
        }
    }

    #[test]
    fn try_trigger_gravity_well_respects_bounds() {
        for _ in 0..500 {
            if let Some(MapEvent::GravityWell {
                center_x, center_y, ..
            }) = try_trigger_random_event(3, 7)
            {
                assert!(center_x < 3);
                assert!(center_y < 7);
            }
        }
    }
}
