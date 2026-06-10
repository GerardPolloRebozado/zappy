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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::storage::World;

    #[test]
    fn test_random_egg() {
        let mut world = World::default();
        let egg1 = world.spawn();
        world.add_component(egg1, Egg);

        let found = Egg::random_egg(&mut world);
        assert_eq!(found, egg1);
    }

    #[test]
    fn test_random_egg_multiple() {
        let mut world = World::default();
        let egg1 = world.spawn();
        world.add_component(egg1, Egg);
        let egg2 = world.spawn();
        world.add_component(egg2, Egg);

        let found = Egg::random_egg(&mut world);
        assert!(found == egg1 || found == egg2);
    }

    #[test]
    #[should_panic(expected = "No eggs found in the world")]
    fn test_random_egg_empty() {
        let mut world = World::default();
        // Register Egg storage by adding a component (which we then remove or just register explicitly)
        let ent = world.spawn();
        world.add_component(ent, Egg);
        world.despawn(ent);

        Egg::random_egg(&mut world);
    }
}
