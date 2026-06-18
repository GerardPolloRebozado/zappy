use log::error;

use crate::{
    ecs::{
        components::{egg::Egg, inhabitant_tag::InhabitantTag, position::Position},
        storage::{Entity, World},
        systems::task::broadcast_event,
    },
    protocol::{Response, ResponseCode, ServerEvent, StatusCode::Ok},
    utils::orientation::RelativeOrientation,
};

pub fn eject(world: &mut World, entity: Entity) -> (Response, Option<ServerEvent>) {
    let executor_position = world.get_component::<Position>(entity).unwrap().clone();
    let executor_orientation = world.get_component::<RelativeOrientation>(entity).unwrap();
    let mut inhabitants_in_tile = Vec::new();
    let mut new_pos = Position {
        x: executor_position.x,
        y: executor_position.y,
    };

    match executor_orientation {
        RelativeOrientation::Forward => new_pos.y -= 1,
        RelativeOrientation::Right => new_pos.x += 1,
        RelativeOrientation::Back => new_pos.y += 1,
        RelativeOrientation::Left => new_pos.x -= 1,
        _ => error!("Orientation on player should never be diagonal"),
    }

    for (e, _) in world.get_storage::<InhabitantTag>().unwrap().iter() {
        let pos = world.get_component::<Position>(*e).unwrap();
        if *pos != executor_position {
            continue;
        }
        inhabitants_in_tile.push(*e);
    }

    for i in inhabitants_in_tile {
        if i == entity {
            continue;
        }
        let pos = world.get_component_mut::<Position>(i).unwrap();
        *pos = new_pos.clone();
    }

    let mut dead_eggs = Vec::new();

    if let Some(egg_storage) = world.get_storage::<Egg>() {
        for (entity, _) in egg_storage.iter() {
            if *world.get_component::<Position>(*entity).unwrap() == executor_position {
                dead_eggs.push(*entity);
                // TODO: GUI doesnt process eject correctly
            }
        }

        for entity in dead_eggs.iter() {
            world.despawn(*entity);
            broadcast_event(
                world,
                ServerEvent::EggDeath {
                    egg_id: entity.id(),
                },
            );
        }
    }

    (
        Response::new(ResponseCode::Status(Ok), None),
        Some(ServerEvent::Eject {
            player_id: entity.id(),
            x: executor_position.x,
            y: executor_position.y,
        }),
    )
}

#[cfg(test)]
mod tests {
    use crate::{
        ecs::builders::inhabitants::build_inhabitant_with_entity,
        utils::orientation::RelativeOrientation,
    };

    use super::*;

    #[test]
    fn eject_player_facing_north() {
        let mut world = World::default();
        let entity = world.spawn();
        build_inhabitant_with_entity(entity, 1, 1, RelativeOrientation::Forward, &mut world);
        let inhabitant2 = world.spawn();
        build_inhabitant_with_entity(inhabitant2, 1, 1, RelativeOrientation::Forward, &mut world);

        let (response, event) = eject(&mut world, inhabitant2);
        assert_eq!(response.code, ResponseCode::Status(Ok));
        let inhabitant2_pos = world.get_component::<Position>(inhabitant2).unwrap();
        assert_eq!(*inhabitant2_pos, Position { x: 1, y: 0 });
        assert!(event.is_some());
    }

    #[test]
    fn eject_player_facing_east() {
        let mut world = World::default();
        let ejector = world.spawn();
        build_inhabitant_with_entity(ejector, 3, 3, RelativeOrientation::Right, &mut world);
        let victim = world.spawn();
        build_inhabitant_with_entity(victim, 3, 3, RelativeOrientation::Forward, &mut world);

        eject(&mut world, ejector);
        let pos = world.get_component::<Position>(victim).unwrap();
        assert_eq!(*pos, Position { x: 4, y: 3 });
    }

    #[test]
    fn eject_player_facing_south() {
        let mut world = World::default();
        let ejector = world.spawn();
        build_inhabitant_with_entity(ejector, 3, 3, RelativeOrientation::Back, &mut world);
        let victim = world.spawn();
        build_inhabitant_with_entity(victim, 3, 3, RelativeOrientation::Forward, &mut world);

        eject(&mut world, ejector);
        let pos = world.get_component::<Position>(victim).unwrap();
        assert_eq!(*pos, Position { x: 3, y: 4 });
    }

    #[test]
    fn eject_player_facing_west() {
        let mut world = World::default();
        let ejector = world.spawn();
        build_inhabitant_with_entity(ejector, 3, 3, RelativeOrientation::Left, &mut world);
        let victim = world.spawn();
        build_inhabitant_with_entity(victim, 3, 3, RelativeOrientation::Forward, &mut world);

        eject(&mut world, ejector);
        let pos = world.get_component::<Position>(victim).unwrap();
        assert_eq!(*pos, Position { x: 2, y: 3 });
    }
}
