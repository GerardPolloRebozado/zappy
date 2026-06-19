use crate::{
    ecs::{
        components::{
            inhabitant_tag::InhabitantTag, inventory::Inventory, level::Level, position::Position,
            resource::Resource::Food, task::TaskList, team::Team,
        },
        storage::{Entity, World},
    },
    utils::{constants::STARTING_FOOD, orientation::RelativeOrientation},
};

pub fn build_inhabitant_with_entity(
    entity: Entity,
    x: u32,
    y: u32,
    orientation: RelativeOrientation,
    world: &mut World,
) -> Entity {
    let mut inv = Inventory::new().with_last_time_consumed(world.current_time);
    inv.add_item(Food, STARTING_FOOD);
    world.add_component(entity, TaskList::default());
    world.add_component(entity, Position::new());
    world.add_component(entity, Level::new());
    world.add_component(entity, inv);
    world.add_component(entity, InhabitantTag);
    world.add_component(entity, RelativeOrientation::Forward);
    world.add_component(entity, Team::default());

    let position = world.get_component_mut::<Position>(entity).unwrap();
    position.x = x;
    position.y = y;

    let mut _orientation = world
        .get_component_mut::<RelativeOrientation>(entity)
        .unwrap();
    *_orientation = orientation;

    entity
}
