//! Passive duration effect for the [`MapEvent::SolarFlare`](crate::ecs::map_events::MapEvent::SolarFlare) anomaly.
//!
//! Gameplay modifiers are read elsewhere via [`MapEvent::modifiers`](crate::ecs::map_events::MapEvent::modifiers).

use crate::ecs::{map_events::MapEvent, storage::World};

/// Advances the solar flare by one tick and returns `true` when the event duration has elapsed.
pub fn tick(world: &mut World) -> bool {
    let MapEvent::SolarFlare {
        ref mut remaining_ticks,
    } = world.map_event
    else {
        return false;
    };

    *remaining_ticks = remaining_ticks.saturating_sub(1);
    *remaining_ticks == 0
}
