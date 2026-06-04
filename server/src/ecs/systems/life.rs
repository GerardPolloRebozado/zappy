use crate::ecs::{
    components::{
        life::Life,
        task::{Task, TaskList, TaskType},
    },
    storage::{Entity, World},
};

/// system that loops throught all lifes and check if the dead_on is older than the current date, if it is then the inhabitant is dead and a dead task is issued
pub fn check_dead_inhabitants(world: &mut World) {
    let mut dead_entities: Vec<Entity> = Vec::new();

    if let Some(storage) = world.get_storage::<Life>() {
        for (entity, life) in storage.iter() {
            if !life.is_alive() {
                dead_entities.push(*entity);
            }
        }
    }

    for entity in dead_entities {
        if let Some(tasks) = world.get_component_mut::<TaskList>(entity) {
            if tasks
                .vector
                .iter()
                .any(|t| matches!(t.task_type, TaskType::Death))
            {
                continue;
            }
            tasks.vector.clear();
            tasks.vector.push(Task {
                task_type: TaskType::Death,
                finish_on: 0,
            });
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::game::Date;

    #[test]
    fn test_life_is_alive() {
        let freq = 100;
        let mut life = Life::new(freq);

        assert!(life.is_alive());

        life.death_on = Date::now().to_timestamp() - 1;
        assert!(!life.is_alive());
    }

    #[test]
    fn test_check_dead_inhabitants_queues_death() {
        let mut world = World::new(100);
        let entity = world.spawn();

        let life = Life {
            death_on: Date::now().to_timestamp() - 1,
        };
        world.add_component(entity, life);

        let mut task_list = TaskList::default();
        task_list.vector.push(Task {
            task_type: TaskType::Forward,
            finish_on: 0,
        });
        world.add_component(entity, task_list);

        check_dead_inhabitants(&mut world);

        let updated_tasks = world.get_component::<TaskList>(entity).unwrap();
        assert_eq!(updated_tasks.vector.len(), 1);
        assert!(matches!(updated_tasks.vector[0].task_type, TaskType::Death));
    }

    #[test]
    fn test_check_dead_inhabitants_no_duplicate_death() {
        let mut world = World::new(100);
        let entity = world.spawn();

        world.add_component(
            entity,
            Life {
                death_on: Date::now().to_timestamp() - 1,
            },
        );

        let mut task_list = TaskList::default();
        task_list.vector.push(Task {
            task_type: TaskType::Death,
            finish_on: 0,
        });
        world.add_component(entity, task_list);

        check_dead_inhabitants(&mut world);

        let updated_tasks = world.get_component::<TaskList>(entity).unwrap();
        assert_eq!(updated_tasks.vector.len(), 1);
    }
}
