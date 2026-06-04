use crate::{
    ecs::{storage::World, systems::task::any_finished_task},
    protocol::Response,
};

pub fn run_systems(world: &mut World) -> Vec<(String, Response)> {
    let responses = Vec::new();

    {
        any_finished_task(world);
    }
    responses
}
