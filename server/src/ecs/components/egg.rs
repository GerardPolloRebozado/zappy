use crate::ecs::storage::{Entity, World};

pub struct Egg {
    pub team: String,
}

impl Egg {
    /// Gets a random egg from the world and returns its entity
    pub fn egg_from_team(world: &mut World, team: String) -> Option<Entity> {
        let egg_storage = world.get_storage::<Egg>().unwrap();

        for (entity, egg) in egg_storage.iter() {
            if egg.team == team {
                return Some(*entity);
            }
        }
        None
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
        world.add_component(
            egg1,
            Egg {
                team: "test".to_string(),
            },
        );

        let found = Egg::egg_from_team(&mut world, "test".to_string());
        assert_eq!(found.unwrap(), egg1);
    }

    #[test]
    fn test_random_egg_multiple() {
        let mut world = World::default();
        let egg1 = world.spawn();
        world.add_component(
            egg1,
            Egg {
                team: "test".to_string(),
            },
        );
        let egg2 = world.spawn();
        world.add_component(
            egg2,
            Egg {
                team: "test".to_string(),
            },
        );

        let found = Egg::egg_from_team(&mut world, "test".to_string());
        assert!(found.unwrap() == egg1 || found.unwrap() == egg2);
    }

    #[test]
    fn test_random_egg_empty() {
        let mut world = World::default();
        let ent = world.spawn();
        world.add_component(
            ent,
            Egg {
                team: "test".to_string(),
            },
        );
        world.despawn(ent);

        assert_eq!(
            Egg::egg_from_team(&mut world, "shold fail".to_string()),
            None
        );
    }
}
