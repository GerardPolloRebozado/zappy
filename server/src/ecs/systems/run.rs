use crate::ecs::{
    storage::World,
    systems::{
        life::check_dead_inhabitants, resource_spawn::resource_spawn_system,
        task::any_finished_task,
    },
};

pub fn run_systems(world: &mut World) {
    check_dead_inhabitants(world);
    any_finished_task(world);
    resource_spawn_system(world);
}
