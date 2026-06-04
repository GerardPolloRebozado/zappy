use crate::ecs::{
    storage::World,
    systems::{life::check_dead_inhabitants, task::any_finished_task},
};

pub fn run_systems(world: &mut World) {
    check_dead_inhabitants(world);
    any_finished_task(world);
}
