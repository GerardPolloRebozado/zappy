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

#[cfg(test)]
mod tests {
    use crate::ecs::components::task::{Task, TaskType};

    use super::*;

    #[test]
    fn add_task() {
        let mut world = World::new();
        world.register_component::<TaskList>();
        let mut entity = world.spawn();
        world.add_component(entity, TaskList::default());

        let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
        let task = Task {
            task_type: TaskType::Forward,
            finish_on: TaskType::Forward.duration() + Date::now().to_timestamp(),
        };
        task_list.vector.push(task);
        assert_eq!(task_list.vector.len(), 1);
    }

    #[test]
    fn task_completion() {
        let mut world = World::new();
        world.register_component::<TaskList>();
        let entity = world.spawn();
        world.add_component(entity, TaskList::default());

        // setup task creation
        {
            let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
            let task = Task {
                task_type: TaskType::Forward,
                finish_on: TaskType::Forward.duration() + Date::now().to_timestamp(),
            };
            task_list.vector.push(task);
        }

        // task should not be finished
        any_finished_task(&mut world, 1);
        {
            let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
            assert_eq!(task_list.vector.len(), 1);
            task_list.vector[0].finish_on = Date::now().to_timestamp() - 1;
        }

        // task should be finished
        any_finished_task(&mut world, 1);
        {
            let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
            assert_eq!(task_list.vector.len(), 0);
        }
    }
}
