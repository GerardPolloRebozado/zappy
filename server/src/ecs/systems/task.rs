//! Task queue processing for queued player commands.
//!
//! Each [`TaskList`] holds a FIFO of timed actions. This module advances timers,
//! pops finished entries, applies their game effects, and returns protocol
//! responses keyed by client UUID.

use crate::{
    ecs::{
        components::{
            inventory::Inventory,
            level::Level,
            position::Position,
            task::{TASK_NOT_STARTED, TaskList, TaskType},
            tile::Tile,
        },
        storage::{Entity, World},
    },
    game::Date,
    protocol::{Response, ResponseCode, StatusCode},
    utils::orientation::RelativeOrientation,
};

/// Advances queued tasks, applies effects when a timer elapses, starts the next
/// task's timer, and returns `(client_uuid, response)` pairs.
///
/// TaskList storage is processed inside `{ ... }` so that borrow ends before
/// `execute_task` mutates other components on the same `world`.
pub fn any_finished_task(world: &mut World, freq: u32) -> Vec<(String, Response)> {
    let mut responses: Vec<(String, Response)> = Vec::new();
    let mut completed: Vec<(Entity, TaskType, Option<String>)> = Vec::new();

    {
        let task_lists = match world.get_storage_mut::<TaskList>() {
            Some(tl) => tl,
            None => return responses,
        };

        for (entity, task_list) in task_lists.iter_mut() {
            let first_task = match task_list.vector.first_mut() {
                Some(t) => t,
                None => continue,
            };

            if first_task.finish_on == TASK_NOT_STARTED {
                first_task.finish_on = Date::now().to_timestamp()
                    + (first_task.task_type.duration() / u64::from(freq));
                continue;
            }

            if !first_task.is_finished() {
                continue;
            }

            completed.push((
                *entity,
                first_task.task_type.clone(),
                task_list.client_uuid.clone(),
            ));

            task_list.vector.remove(0);

            if let Some(new_first_task) = task_list.vector.first_mut() {
                new_first_task.finish_on = Date::now().to_timestamp()
                    + (new_first_task.task_type.duration() / u64::from(freq));
            }
        }
    }

    for (entity, task_type, client_uuid) in completed {
        let response = execute_task(world, entity, &task_type);
        if let Some(uuid) = client_uuid {
            responses.push((uuid, response));
        }
    }

    responses
}

/// Applies the game effect of a completed task on `entity`.
///
/// `Forward` updates [`Position`] from [`RelativeOrientation`] on the same entity.
/// `TurnRight` / `TurnLeft` rotate that orientation.
fn execute_task(world: &mut World, entity: Entity, task_type: &TaskType) -> Response {
    match task_type {
        TaskType::Forward => {
            let map_width = world.mapSize.width;
            let map_height = world.mapSize.height;
            let orientation = world.get_component::<RelativeOrientation>(entity).copied();
            if let Some(pos) = world.get_component_mut::<Position>(entity) {
                if let Some(ori) = orientation {
                    pos.move_forward(ori, map_width, map_height);
                }
            }
            Response::new(ResponseCode::Status(StatusCode::Ok), None)
        }
        TaskType::TurnRight => {
            if let Some(ori) = world.get_component_mut::<RelativeOrientation>(entity) {
                *ori = ori.turn_right();
            }
            Response::new(ResponseCode::Status(StatusCode::Ok), None)
        }
        TaskType::TurnLeft => {
            if let Some(ori) = world.get_component_mut::<RelativeOrientation>(entity) {
                *ori = ori.turn_left();
            }
            Response::new(ResponseCode::Status(StatusCode::Ok), None)
        }
        TaskType::Look => {
            let data = execute_look(world, entity);
            Response::new(ResponseCode::Status(StatusCode::Ok), Some(data))
        }
        _ => Response::new(ResponseCode::Status(StatusCode::Ok), None),
    }
}

fn execute_look(world: &World, entity: Entity) -> String {
    let pos = match world.get_component::<Position>(entity) {
        Some(p) => p,
        None => return "[]".to_string(),
    };
    let ori = match world.get_component::<RelativeOrientation>(entity) {
        Some(o) => o,
        None => return "[]".to_string(),
    };
    let level = world
        .get_component::<Level>(entity)
        .map(|l| l.value)
        .unwrap_or(1);
    let map_width = world.mapSize.width;
    let map_height = world.mapSize.height;

    let mut tiles_content = Vec::new();

    // TODO: Implement vision modifications from biomes (e.g. Luminous Orchards)
    // TODO: Implement Solar Flare global vision
    for i in 0..=i32::from(level) {
        for j in -i..=i {
            let (f, r) = (i, j);
            let (ax, ay) = match ori {
                RelativeOrientation::Forward => (
                    i32::try_from(pos.x).unwrap() + r,
                    i32::try_from(pos.y).unwrap() - f,
                ),
                RelativeOrientation::ForwardLeft => (
                    i32::try_from(pos.x).unwrap() + f,
                    i32::try_from(pos.y).unwrap() + r,
                ),
                RelativeOrientation::Left => (
                    i32::try_from(pos.x).unwrap() - r,
                    i32::try_from(pos.y).unwrap() + f,
                ),
                RelativeOrientation::BackLeft => (
                    i32::try_from(pos.x).unwrap() - f,
                    i32::try_from(pos.y).unwrap() - r,
                ),
                _ => (i32::try_from(pos.x).unwrap(), i32::try_from(pos.y).unwrap()),
            };
            let x = u32::try_from(ax.rem_euclid(i32::try_from(map_width).unwrap())).unwrap();
            let y = u32::try_from(ay.rem_euclid(i32::try_from(map_height).unwrap())).unwrap();

            tiles_content.push(get_tile_info(world, x, y));
        }
    }

    format!("[{}]", tiles_content.join(","))
}

fn get_tile_info(world: &World, x: u32, y: u32) -> String {
    let mut info = Vec::new();

    // players on this tile
    if let (Some(positions), Some(orientations)) = (
        world.get_storage::<Position>(),
        world.get_storage::<RelativeOrientation>(),
    ) {
        for (ent, pos) in positions.iter() {
            if pos.x == x && pos.y == y && orientations.get(*ent).is_some() {
                info.push("player".to_string());
            }
        }
    }

    // resources on this tile
    if let (Some(tiles), Some(positions), Some(inventories)) = (
        world.get_storage::<Tile>(),
        world.get_storage::<Position>(),
        world.get_storage::<Inventory>(),
    ) {
        for (ent, _tile) in tiles.iter() {
            if let Some(pos) = positions.get(*ent) {
                if pos.x == x && pos.y == y {
                    if let Some(inv) = inventories.get(*ent) {
                        let mut resources: Vec<String> = Vec::new();
                        for (res, count) in &inv.items {
                            for _ in 0..*count {
                                resources.push(res.to_string());
                            }
                        }
                        // sort resources to have a consistent output
                        resources.sort();
                        info.extend(resources);
                    }
                    break;
                }
            }
        }
    }

    info.join(" ")
}

#[cfg(test)]
mod tests {
    use crate::ecs::components::task::{Task, TaskType};
    use crate::ecs::storage::World;

    use super::*;

    fn setup_inhabitant(
        x: u32,
        y: u32,
        orientation: RelativeOrientation,
        map_w: u32,
        map_h: u32,
    ) -> (World, Entity) {
        let mut world = World::new();
        world.mapSize.width = map_w;
        world.mapSize.height = map_h;
        world.register_component::<Position>();
        world.register_component::<RelativeOrientation>();
        world.register_component::<Level>();
        world.register_component::<TaskList>();
        world.register_component::<Inventory>();
        let entity = world.spawn();
        world.add_component(entity, Position { x, y });
        world.add_component(entity, orientation);
        world.add_component(entity, Level::new());
        world.add_component(entity, TaskList::default());
        world.add_component(entity, Inventory::new());
        (world, entity)
    }

    fn assert_ok(response: Response) {
        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));
        assert!(response.data.is_none());
    }
    #[test]
    fn execute_task_forward_moves_north() {
        let (mut world, entity) = setup_inhabitant(5, 5, RelativeOrientation::Forward, 10, 10);
        let response = execute_task(&mut world, entity, &TaskType::Forward);
        assert_ok(response);
        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (5, 4));
    }

    #[test]
    fn execute_task_forward_moves_east() {
        let (mut world, entity) = setup_inhabitant(5, 5, RelativeOrientation::ForwardLeft, 10, 10);
        execute_task(&mut world, entity, &TaskType::Forward);
        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (6, 5));
    }

    #[test]
    fn execute_task_forward_wraps() {
        let (mut world, entity) = setup_inhabitant(3, 0, RelativeOrientation::Forward, 10, 10);
        execute_task(&mut world, entity, &TaskType::Forward);
        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (3, 9));
    }

    #[test]
    fn execute_task_forward_without_orientation() {
        let mut world = World::new();
        world.mapSize.width = 10;
        world.mapSize.height = 10;
        world.register_component::<Position>();
        let entity = world.spawn();
        world.add_component(entity, Position { x: 5, y: 5 });

        execute_task(&mut world, entity, &TaskType::Forward);

        let pos = world.get_component::<Position>(entity).unwrap();
        assert_eq!((pos.x, pos.y), (5, 5));
    }

    #[test]
    fn execute_task_turn_right() {
        let (mut world, entity) = setup_inhabitant(0, 0, RelativeOrientation::Forward, 10, 10);
        execute_task(&mut world, entity, &TaskType::TurnRight);
        let ori = world.get_component::<RelativeOrientation>(entity).unwrap();
        assert_eq!(*ori, RelativeOrientation::ForwardLeft);
    }

    #[test]
    fn execute_task_turn_left() {
        let (mut world, entity) = setup_inhabitant(0, 0, RelativeOrientation::Forward, 10, 10);
        execute_task(&mut world, entity, &TaskType::TurnLeft);
        let ori = world.get_component::<RelativeOrientation>(entity).unwrap();
        assert_eq!(*ori, RelativeOrientation::BackLeft);
    }

    #[test]
    fn execute_task_turn_without_orientation() {
        let mut world = World::new();
        world.register_component::<Position>();
        let entity = world.spawn();
        world.add_component(entity, Position { x: 1, y: 1 });

        let response = execute_task(&mut world, entity, &TaskType::TurnRight);
        assert_ok(response);
    }

    #[test]
    fn execute_task_look() {
        let mut world = World::new();
        world.mapSize.width = 10;
        world.mapSize.height = 10;
        world.register_component::<Position>();
        world.register_component::<RelativeOrientation>();
        world.register_component::<Level>();
        world.register_component::<Tile>();
        world.register_component::<Inventory>();

        let entity = world.spawn();
        world.add_component(entity, Position { x: 5, y: 5 });
        world.add_component(entity, RelativeOrientation::Forward);
        world.add_component(entity, Level { value: 1 });

        // Add a resource on tile (5, 4) - which is in front of (5, 5) facing North
        let tile = world.spawn();
        world.add_component(tile, Tile);
        world.add_component(tile, Position { x: 5, y: 4 });
        let mut inv = Inventory::new();
        inv.items.insert(crate::game::Resource::Food, 1);
        world.add_component(tile, inv);

        let response = execute_task(&mut world, entity, &TaskType::Look);
        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));
        let data = response.data.unwrap();
        // Level 1 vision: 4 tiles.
        // Tile 0: (5,5) -> "player"
        // Tile 1: (4,4) -> ""
        // Tile 2: (5,4) -> "food"
        // Tile 3: (6,4) -> ""
        // Note: players on the same tile are also listed. Our player is at (5,5).
        assert_eq!(data, "[player,,food,]");
    }

    #[test]
    fn execute_task_unimplemented_returns_ok() {
        let (mut world, entity) = setup_inhabitant(2, 3, RelativeOrientation::Forward, 10, 10);
        let before = (
            world.get_component::<Position>(entity).unwrap().x,
            world.get_component::<Position>(entity).unwrap().y,
            *world.get_component::<RelativeOrientation>(entity).unwrap(),
        );

        let response = execute_task(&mut world, entity, &TaskType::Inventory);
        assert_ok(response);

        let after = (
            world.get_component::<Position>(entity).unwrap().x,
            world.get_component::<Position>(entity).unwrap().y,
            *world.get_component::<RelativeOrientation>(entity).unwrap(),
        );
        assert_eq!(before, after);
    }

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
