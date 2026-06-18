use crate::ecs::{
    components::{
        inhabitant_tag::InhabitantTag,
        inventory::Inventory,
        resource::Resource::Food,
        task::{Task, TaskList, TaskType},
    },
    storage::{Entity, World},
};

/// system that loops throught all inhabitant invetories and check if the dead_on is older than the current date, if it is then the inhabitant is dead and a dead task is issued
pub fn check_dead_inhabitants(world: &mut World) {
    let mut dead_entities: Vec<Entity> = Vec::new();
    let mut inhabitants = Vec::new();

    if let Some(storage) = world.get_storage::<InhabitantTag>() {
        for (entity, _) in storage.iter() {
            inhabitants.push(*entity);
        }
    }

    for entity in inhabitants {
        let freq = world.freq;
        let now = world.current_time;

        let inv = match world.get_component_mut::<Inventory>(entity) {
            Some(inv) => inv,
            None => continue,
        };

        inv.consume_food(freq, now);

        if inv.get_item_count(Food) == 0 {
            dead_entities.push(entity);
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
