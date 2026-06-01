use crate::{
    ecs::{components::task::TaskList, storage::World},
    game::Date,
};

pub fn any_finished_task(world: &mut World, freq: u32) {
    let task_lists = world.get_storage_mut::<TaskList>();
    if task_lists.is_none() {
        return;
    }
    let task_lists = task_lists.unwrap();

    for (_, task_list) in task_lists.iter_mut() {
        let first_task = task_list.vector.first();
        if first_task.is_none() {
            continue;
        }
        let first_task = first_task.unwrap();

        if first_task.is_finished() {
            task_list.vector.remove(0);
            if let Some(first_task) = task_list.vector.first_mut() {
                first_task.finish_on =
                    Date::now().to_timestamp() + (first_task.task_type.duration() / u64::from(freq))
            }
        }
    }
}
