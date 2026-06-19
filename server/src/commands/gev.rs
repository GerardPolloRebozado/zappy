use crate::ecs::{map_events::MapEvent, storage::World, systems::map_event::event_name};

/// Builds the response line for a `gev` query.
///
/// Returns `"gev none"` when no event is active, or `"gev <name>"` for most
/// events. Gravity wells include the center coordinates: `"gev gravity_well X Y"`.
pub fn build_gev_line(world: &World) -> String {
    let event = &world.map_event;
    let name = event_name(event);
    if let MapEvent::GravityWell {
        center_x, center_y, ..
    } = event
    {
        format!("gev {name} {center_x} {center_y}")
    } else {
        format!("gev {name}")
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::storage::World;

    #[test]
    fn build_gev_line_no_event() {
        let world = World::default();
        assert_eq!(build_gev_line(&world), "gev none");
    }

    #[test]
    fn build_gev_line_solar_flare() {
        let mut world = World::default();
        world.map_event = MapEvent::new_solar_flare();
        assert_eq!(build_gev_line(&world), "gev solar_flare");
    }

    #[test]
    fn build_gev_line_meteor_shower() {
        let mut world = World::default();
        world.map_event = MapEvent::new_meteor_shower();
        assert_eq!(build_gev_line(&world), "gev meteor_shower");
    }

    #[test]
    fn build_gev_line_gravity_well_includes_coords() {
        let mut world = World::default();
        world.map_event = MapEvent::new_gravity_well(5, 7);
        assert_eq!(build_gev_line(&world), "gev gravity_well 5 7");
    }
}
