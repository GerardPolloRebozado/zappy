use log::info;

use crate::{
    ecs::{
        map_events::{EVENT_TRIGGER_INTERVAL, MapEvent, try_trigger_random_event},
        storage::World,
        systems::{
            map_events::{gravity_well, meteor_shower, psionic_echo, solar_flare},
            task::broadcast_event,
        },
    },
    protocol::ServerEvent,
    utils::date::Date,
};

/// Returns a human-readable name for the event, used in `smg` broadcasts.
fn event_name(event: &MapEvent) -> &'static str {
    match event {
        MapEvent::None => "none",
        MapEvent::MeteorShower { .. } => "meteor_shower",
        MapEvent::SolarFlare { .. } => "solar_flare",
        MapEvent::GravityWell { .. } => "gravity_well",
        MapEvent::PsionicEcho => "psionic_echo",
    }
}

/// Core map-event system. Called every tick from [`super::run::run_systems`].
///
/// When no event is active it periodically rolls a random trigger check.
/// When an event is active it ticks down and applies per-tick effects
/// (gem spawns, gravity pulls, incantation acceleration, etc.).
pub fn map_event_system(world: &mut World) {
    let now = Date::now().to_timestamp();
    let freq = world.freq;

    if !world.map_event.is_active() {
        try_trigger_check(world, now, freq);
    } else {
        tick_active_event(world, now, freq);
    }
}

/// Rolls a random event trigger if enough time has passed since the last check.
fn try_trigger_check(world: &mut World, now: u64, freq: u64) {
    let ms_between_checks = (EVENT_TRIGGER_INTERVAL as u128 * 1000) / freq as u128;
    if now - world.last_event_trigger_check < ms_between_checks as u64 {
        return;
    }
    world.last_event_trigger_check = now;

    let width = world.map_size.width;
    let height = world.map_size.height;
    if let Some(event) = try_trigger_random_event(width, height) {
        let name = event_name(&event);
        info!("Map event triggered: {}", name);

        // PsionicEcho is run instantly and never stored in world.map_event
        if matches!(event, MapEvent::PsionicEcho) {
            psionic_echo::apply_psionic_echo(world, now);
            broadcast_event(
                world,
                ServerEvent::ServerMessage {
                    message: format!("event_start {name}"),
                },
            );
            broadcast_event(
                world,
                ServerEvent::ServerMessage {
                    message: format!("event_end {name}"),
                },
            );
        } else {
            world.map_event = event;
            world.last_event_check = now;
            broadcast_event(
                world,
                ServerEvent::ServerMessage {
                    message: format!("event_start {name}"),
                },
            );
        }
    }
}

/// Advances the currently active [`MapEvent`] by one time unit when enough
/// real time has passed since [`World::last_event_check`].
///
/// Per-event tick logic lives in [`super::map_events`]. When a tick reports
/// that the event duration has elapsed, [`expire_event`] clears it and
/// broadcasts an end notification.
fn tick_active_event(world: &mut World, now: u64, freq: u64) {
    let ms_per_tick = (1000_u128) / freq as u128;
    if now - world.last_event_check < ms_per_tick as u64 {
        return;
    }
    world.last_event_check = now;

    let expired = match world.map_event {
        MapEvent::MeteorShower { .. } => meteor_shower::tick(world),
        MapEvent::SolarFlare { .. } => solar_flare::tick(world),
        MapEvent::GravityWell { .. } => gravity_well::tick(world),
        MapEvent::PsionicEcho | MapEvent::None => false,
    };

    if expired {
        expire_event(world);
    }
}

/// Sets the active event to `None` and broadcasts an end notification.
fn expire_event(world: &mut World) {
    let name = event_name(&world.map_event);
    info!("Map event expired: {}", name);
    world.map_event = MapEvent::None;
    broadcast_event(
        world,
        ServerEvent::ServerMessage {
            message: format!("event_end {name}"),
        },
    );
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::map_events::{GRAVITY_WELL_PULL_INTERVAL, METEOR_SHOWER_SPAWN_INTERVAL};
    use crate::ecs::{
        builders::tile::build_tile,
        components::{
            inhabitant_tag::InhabitantTag,
            position::Position,
            resource::Resource,
            task::{Task, TaskList, TaskType},
            terrain_type::TerrainType,
        },
        map_events::MapEvent,
        storage::World,
    };
    use crate::utils::date::Date;

    fn world_with_tiles(width: u32, height: u32) -> World {
        let mut world = World::new(crate::ecs::map_size::MapSize { width, height }, 100);
        for y in 0..height {
            for x in 0..width {
                build_tile(Position { x, y }, &mut world, TerrainType::Grass);
            }
        }
        world
    }

    #[test]
    fn no_event_does_not_crash() {
        let mut world = World::default();
        world.map_event = MapEvent::None;
        map_event_system(&mut world);
        assert_eq!(world.map_event, MapEvent::None);
    }

    #[test]
    fn meteor_shower_tick_down_to_expiry() {
        let mut world = world_with_tiles(5, 5);
        world.map_event = MapEvent::MeteorShower {
            remaining_ticks: 1,
            ticks_until_next_spawn: 10,
        };
        world.last_event_check = 0;
        map_event_system(&mut world);
        assert_eq!(world.map_event, MapEvent::None);
    }

    #[test]
    fn solar_flare_expires() {
        let mut world = World::default();
        world.map_event = MapEvent::SolarFlare { remaining_ticks: 1 };
        world.last_event_check = 0;
        map_event_system(&mut world);
        assert_eq!(world.map_event, MapEvent::None);
    }

    #[test]
    fn gravity_well_expires() {
        let mut world = World::default();
        world.map_event = MapEvent::GravityWell {
            remaining_ticks: 1,
            ticks_until_next_pull: 10,
            center_x: 0,
            center_y: 0,
        };
        world.last_event_check = 0;
        map_event_system(&mut world);
        assert_eq!(world.map_event, MapEvent::None);
    }

    #[test]
    fn meteor_shower_spawns_gems_when_spawn_timer_fires() {
        let mut world = world_with_tiles(10, 10);
        world.map_event = MapEvent::MeteorShower {
            remaining_ticks: 10,
            ticks_until_next_spawn: 1,
        };
        world.last_event_check = 0;

        let before: u32 = Resource::iter()
            .filter(|r| *r != Resource::Food)
            .map(|r| world.resources_amount.get_item_count(r))
            .sum();

        map_event_system(&mut world);

        let after: u32 = Resource::iter()
            .filter(|r| *r != Resource::Food)
            .map(|r| world.resources_amount.get_item_count(r))
            .sum();

        assert!(after > before, "meteor shower should have spawned gems");
        assert!(world.map_event.is_active(), "event should still be active");
    }

    #[test]
    fn meteor_shower_resets_spawn_timer_after_spawn() {
        let mut world = world_with_tiles(10, 10);
        world.map_event = MapEvent::MeteorShower {
            remaining_ticks: 10,
            ticks_until_next_spawn: 1,
        };
        world.last_event_check = 0;

        map_event_system(&mut world);

        match world.map_event {
            MapEvent::MeteorShower {
                ticks_until_next_spawn,
                ..
            } => assert_eq!(
                ticks_until_next_spawn, METEOR_SHOWER_SPAWN_INTERVAL,
                "spawn timer should reset"
            ),
            _ => panic!("expected MeteorShower to remain active"),
        }
    }

    #[test]
    fn gravity_well_pulls_inhabitant_closer() {
        let mut world = world_with_tiles(10, 10);
        let entity = world.spawn();
        world.add_component(entity, InhabitantTag);
        world.add_component(entity, Position { x: 3, y: 3 });

        world.map_event = MapEvent::GravityWell {
            remaining_ticks: 10,
            ticks_until_next_pull: 1,
            center_x: 5,
            center_y: 5,
        };
        world.last_event_check = 0;

        map_event_system(&mut world);

        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!(pos.x, 4, "should move one step toward center_x=5");
        assert_eq!(pos.y, 4, "should move one step toward center_y=5");
    }

    #[test]
    fn gravity_well_toroidal_pull() {
        let mut world = world_with_tiles(10, 10);
        let entity = world.spawn();
        world.add_component(entity, InhabitantTag);
        world.add_component(entity, Position { x: 1, y: 1 });

        world.map_event = MapEvent::GravityWell {
            remaining_ticks: 10,
            ticks_until_next_pull: 1,
            center_x: 9,
            center_y: 9,
        };
        world.last_event_check = 0;

        map_event_system(&mut world);

        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!(pos.x, 0, "should wrap around toward center_x=9 via x=0");
        assert_eq!(pos.y, 0, "should wrap around toward center_y=9 via y=0");
    }

    #[test]
    fn gravity_well_resets_pull_timer() {
        let mut world = world_with_tiles(10, 10);
        world.map_event = MapEvent::GravityWell {
            remaining_ticks: 10,
            ticks_until_next_pull: 1,
            center_x: 5,
            center_y: 5,
        };
        world.last_event_check = 0;

        map_event_system(&mut world);

        match world.map_event {
            MapEvent::GravityWell {
                ticks_until_next_pull,
                ..
            } => assert_eq!(
                ticks_until_next_pull, GRAVITY_WELL_PULL_INTERVAL,
                "pull timer should reset"
            ),
            _ => panic!("expected GravityWell to remain active"),
        }
    }

    #[test]
    fn psionic_echo_reduces_incantation_time() {
        let mut world = World::default();
        let now = Date::now().to_timestamp();
        let finish_on = now + 1000;

        let entity = world.spawn();
        world.add_component(
            entity,
            TaskList {
                vector: vec![Task {
                    task_type: TaskType::Incantation,
                    finish_on,
                }],
            },
        );

        psionic_echo::apply_psionic_echo(&mut world, now);

        let task_list = world.get_component::<TaskList>(entity).unwrap();
        let expected = finish_on - (finish_on - now) / 4;
        assert_eq!(task_list.vector[0].finish_on, expected);
    }

    #[test]
    fn solar_flare_tick_decrements_without_side_effects() {
        let mut world = World::default();
        world.map_event = MapEvent::SolarFlare { remaining_ticks: 5 };
        world.last_event_check = 0;

        map_event_system(&mut world);

        match world.map_event {
            MapEvent::SolarFlare { remaining_ticks } => {
                assert_eq!(remaining_ticks, 4, "should decrement by 1");
            }
            _ => panic!("expected SolarFlare to remain active"),
        }
    }

    #[test]
    fn event_not_ticked_if_insufficient_time_elapsed() {
        let mut world = World::default();
        let now = Date::now().to_timestamp();
        world.map_event = MapEvent::SolarFlare { remaining_ticks: 5 };
        world.last_event_check = now;

        map_event_system(&mut world);

        match world.map_event {
            MapEvent::SolarFlare { remaining_ticks } => {
                assert_eq!(remaining_ticks, 5, "should not tick yet");
            }
            _ => panic!("expected SolarFlare to remain active"),
        }
    }
}
