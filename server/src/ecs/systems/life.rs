use crate::ecs::{components::life::Life, storage::World};

pub fn check_dead_inhabitants(world: &mut World) {
    let mut death_entities = Vec::new();

    {
        let storage = world.get_storage::<Life>();
        if storage.is_none() {
            return;
        }
        let storage = storage.unwrap();
        for (entity, life) in storage.iter() {
            if life.is_alive() {
                continue;
            }

            death_entities.push(entity);
        }
    }
}
