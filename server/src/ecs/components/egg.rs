use crate::ecs::storage::{Entity, World};
use rand::prelude::IteratorRandom;
use rand::rng;

pub struct Egg;

impl Egg {
    /// Gets a random egg from the world and returns its entity
    pub fn random_egg(world: &mut World) -> Entity {
        let egg_storage = world.get_storage::<Egg>().unwrap();
        let mut rng = rng();

        if let Some((entity, _)) = egg_storage.iter().choose(&mut rng) {
            *entity
        } else {
            panic!("No eggs found in the world");
        }
    }
}
