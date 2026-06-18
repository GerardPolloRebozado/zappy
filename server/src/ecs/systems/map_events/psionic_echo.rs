use crate::ecs::{
    components::task::{TASK_NOT_STARTED, TaskList, TaskType},
    storage::{Entity, World},
};

/// Reduces all active incantations' remaining time by 25% (PsionicEcho effect).
pub fn apply_psionic_echo(world: &mut World, now: u64) {
    let entities: Vec<Entity> = if let Some(task_storage) = world.get_storage::<TaskList>() {
        task_storage.iter().map(|(e, _)| *e).collect()
    } else {
        return;
    };

    for entity in entities {
        if let Some(task_list) = world.get_component_mut::<TaskList>(entity)
            && let Some(head) = task_list.vector.first_mut()
            && matches!(head.task_type, TaskType::Incantation)
            && head.finish_on != TASK_NOT_STARTED
            && head.finish_on > now
        {
            let remaining = head.finish_on - now;
            head.finish_on -= remaining / 4;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::{components::task::Task, storage::World};
    use crate::utils::date::Date;

    #[test]
    fn reduces_incantation_time() {
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

        apply_psionic_echo(&mut world, now);

        let task_list = world.get_component::<TaskList>(entity).unwrap();
        let expected = finish_on - (finish_on - now) / 4;
        assert_eq!(task_list.vector[0].finish_on, expected);
    }
}
