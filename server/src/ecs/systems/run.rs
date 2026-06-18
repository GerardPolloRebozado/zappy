use crate::ecs::{
    storage::World,
    systems::{
        food::check_dead_inhabitants, map_event::map_event_system,
        resource_spawn::resource_spawn_system, task::any_finished_task,
    },
};

pub fn run_systems(world: &mut World) {
    check_dead_inhabitants(world);
    any_finished_task(world);
    resource_spawn_system(world);
    map_event_system(world);
}
