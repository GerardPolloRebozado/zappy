use crate::{
    ecs::{
        components::task::{TASK_NOT_STARTED, TaskList},
        storage::World,
    },
    game::Date,
    protocol::{Response, ResponseCode, StatusCode},
};

/// System that can be run on every game iteration to check if theres any task finished, if its finished it will remove it and start the next one
pub fn any_finished_task(world: &mut World, freq: u32) -> Vec<(String, Response)> {
    let mut responses: Vec<(String, Response)> = Vec::new();
    let task_lists = world.get_storage_mut::<TaskList>();
    if task_lists.is_none() {
        return responses;
    }
    let task_lists = task_lists.unwrap();

    for (_, task_list) in task_lists.iter_mut() {
        let first_task = match task_list.vector.first_mut() {
            Some(t) => t,
            None => continue,
        };

        if first_task.finish_on == TASK_NOT_STARTED {
            first_task.finish_on =
                Date::now().to_timestamp() + (first_task.task_type.duration() / u64::from(freq));
            continue;
        }

        if !first_task.is_finished() {
            continue;
        }

        if let Some(uuid) = &task_list.client_uuid {
            responses.push((
                uuid.clone(),
                Response::new(ResponseCode::Status(StatusCode::Ok), None),
            ));
        }

        task_list.vector.remove(0);

        if let Some(new_first_task) = task_list.vector.first_mut() {
            new_first_task.finish_on =
                Date::now().to_timestamp() + (new_first_task.task_type.duration() / u64::from(freq))
        }
    }
    responses
}

#[cfg(test)]
mod tests {
    use crate::ecs::components::task::{Task, TaskType};
    use crate::ecs::storage::World;

    use super::*;

    #[test]
    fn add_task() {
        let mut world = World::new();
        world.register_component::<TaskList>();
        let entity = world.spawn();
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
        let _ = any_finished_task(&mut world, 1);
        {
            let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
            assert_eq!(task_list.vector.len(), 1);
            task_list.vector[0].finish_on = Date::now().to_timestamp() - 1;
        }

        // task should be finished
        let _ = any_finished_task(&mut world, 1);
        {
            let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
            assert_eq!(task_list.vector.len(), 0);
        }
    }
}
