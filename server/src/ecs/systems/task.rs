//! Task queue processing for queued player commands.
//!
//! Each [`TaskList`] holds a FIFO of timed actions. This module advances timers,
//! pops finished entries, applies their game effects, and returns protocol
//! responses keyed by client UUID.
//!
//! # Broadcast flow
//!
//! 1. [`crate::server::commands`] queues [`TaskType::BroadcastText`] with the message text.
//! 2. When the timer elapses, [`execute_task`] returns `ok` plus a [`ServerEvent::Message`].
//! 3. [`any_finished_task`] pushes `ok` to the acting client, then calls
//!    [`broadcast_event`] so every AI receives `message k, text`
//!    and the GUI receives `pbc #n text` (see [`ServerEvent::to_ai_string`]).

use crate::{
    ecs::{
        components::{
            network::NetworkData,
            position::Position,
            task::{TASK_NOT_STARTED, TaskList, TaskType},
            team::Team,
        },
        storage::{Entity, World},
    },
    game::{Date, Inhabitant},
    protocol::{Response, ResponseCode, ServerEvent, StatusCode},
    utils::orientation::RelativeOrientation,
};

pub mod look;

/// Advances queued tasks, applies effects when a timer elapses, starts the next
/// task's timer, and handles command replies and broadcast events.
pub fn any_finished_task(world: &mut World) {
    let mut completed: Vec<(Entity, TaskType)> = Vec::new();
    let freq = world.freq;

    {
        let task_lists = match world.get_storage_mut::<TaskList>() {
            Some(tl) => tl,
            None => return,
        };

        for (entity, task_list) in task_lists.iter_mut() {
            let first_task = match task_list.vector.first_mut() {
                Some(t) => t,
                None => continue,
            };

            if first_task.finish_on == TASK_NOT_STARTED {
                first_task.finish_on =
                    Date::now().to_timestamp() + (first_task.task_type.duration() / freq);
                continue;
            }

            if !first_task.is_finished() {
                continue;
            }

            completed.push((*entity, first_task.task_type.clone()));

            task_list.vector.remove(0);

            if let Some(new_first_task) = task_list.vector.first_mut() {
                new_first_task.finish_on =
                    Date::now().to_timestamp() + (new_first_task.task_type.duration() / freq);
            }
        }
    }

    for (entity, task_type) in completed {
        let (response, event) = execute_task(world, entity, &task_type);

        if matches!(task_type, TaskType::Death) {
            if let Some(network_data) = world.get_component_mut::<NetworkData>(entity) {
                use std::io::Write;
                let _ = network_data
                    .socket
                    .write_all(response.to_string().as_bytes());
            }
            world.despawn(entity);
        } else {
            if let Some(network_data) = world.get_component_mut::<NetworkData>(entity) {
                network_data.pending_responses.push(response);
            }
        }

        if let Some(ev) = event {
            broadcast_event(world, ev);
        }
    }
}

/// Pushes a [`ServerEvent`] to every connected client that should see it.
pub fn broadcast_event(world: &mut World, event: ServerEvent) {
    let map_width = world.map_size.width;
    let map_height = world.map_size.height;

    let mut ai_data = Vec::new();
    let mut gui_entities = Vec::new();

    if let Some(team_storage) = world.get_storage::<Team>() {
        for (entity, team) in team_storage.iter() {
            match &team {
                Team::AuthenticatedGUI => gui_entities.push(*entity),
                Team::AuthenticatedAI(_) => {
                    if let Some(inhabitant) = Inhabitant::get(*entity, world)
                        && let Some(line) =
                            event.to_ai_string(Some(&inhabitant), map_width, map_height)
                    {
                        ai_data.push((*entity, line));
                    }
                }
                _ => {}
            }
        }
    }

    for entity in gui_entities {
        if let Some(line) = event.to_gui_string()
            && let Some(nd) = world.get_component_mut::<NetworkData>(entity)
        {
            nd.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(line),
            ));
        }
    }

    for (entity, line) in ai_data {
        if let Some(nd) = world.get_component_mut::<NetworkData>(entity) {
            nd.pending_responses.push(Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(line),
            ));
        }
    }
}

/// Applies the game effect of a completed task on `entity`.
///
/// `Forward` updates [`Position`] from [`RelativeOrientation`] on the same entity.
/// `TurnRight` / `TurnLeft` rotate that orientation.
/// `BroadcastText` produces a [`ServerEvent::Message`] for fan-out.
///
/// Returns the response for the acting client plus an optional broadcast event.
fn execute_task(
    world: &mut World,
    entity: Entity,
    task_type: &TaskType,
) -> (Response, Option<ServerEvent>) {
    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), None);

    match task_type {
        TaskType::Forward => {
            let map_width = world.map_size.width;
            let map_height = world.map_size.height;
            let orientation = world.get_component::<RelativeOrientation>(entity).copied();
            if let Some(pos) = world.get_component_mut::<Position>(entity)
                && let Some(ori) = orientation
            {
                pos.move_forward(ori, map_width, map_height);
            }
            (ok, None)
        }
        TaskType::TurnRight => {
            if let Some(ori) = world.get_component_mut::<RelativeOrientation>(entity) {
                *ori = ori.turn_right();
            }
            (ok, None)
        }
        TaskType::TurnLeft => {
            if let Some(ori) = world.get_component_mut::<RelativeOrientation>(entity) {
                *ori = ori.turn_left();
            }
            (ok, None)
        }
        TaskType::Look => (
            Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some(look::execute_look(world, entity)),
            ),
            None,
        ),
        TaskType::Death => (
            Response::new(
                ResponseCode::Status(StatusCode::Ok),
                Some("dead".to_string()),
            ),
            None,
        ),
        TaskType::BroadcastText(text) => {
            // No ECS mutation: only snapshot position for ServerEvent + later calc_k per receiver.
            let mut event = None;
            if let Some(broadcaster) = Inhabitant::get(entity, world) {
                event = Some(ServerEvent::message(&broadcaster, text.as_str()));
            }
            (ok, event)
        }
        _ => (ok, None),
    }
}

#[cfg(test)]
mod tests {
    use crate::ecs::components::inventory::Inventory;
    use crate::ecs::components::level::Level;
    use crate::ecs::components::life::Life;
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
        let mut world = World::default();
        world.map_size.width = map_w;
        world.map_size.height = map_h;

        let entity = world.spawn();
        world.add_component(entity, Position { x, y });
        world.add_component(entity, orientation);
        world.add_component(entity, Level::new());
        world.add_component(entity, TaskList::default());
        world.add_component(entity, Inventory::new());
        world.add_component(entity, Life::new(world.freq));
        world.add_component(entity, crate::ecs::components::team::Team::default());
        (world, entity)
    }

    fn assert_ok(response: Response) {
        assert_eq!(response.code, ResponseCode::Status(StatusCode::Ok));
        assert!(response.data.is_none());
    }
    #[test]
    fn execute_task_forward_moves_north() {
        let (mut world, entity) = setup_inhabitant(5, 5, RelativeOrientation::Forward, 10, 10);
        let (response, _) = execute_task(&mut world, entity, &TaskType::Forward);
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
        let mut world = World::default();
        world.map_size.width = 10;
        world.map_size.height = 10;
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
        let mut world = World::default();
        let entity = world.spawn();
        world.add_component(entity, Position { x: 1, y: 1 });

        let (response, _) = execute_task(&mut world, entity, &TaskType::TurnRight);
        assert_ok(response);
    }

    #[test]
    fn execute_task_broadcast_returns_message_event() {
        let (mut world, entity) = setup_inhabitant(2, 3, RelativeOrientation::Forward, 10, 10);
        let (response, event) =
            execute_task(&mut world, entity, &TaskType::BroadcastText("hi".into()));
        assert_ok(response);
        assert!(matches!(
            event,
            Some(ServerEvent::Message {
                message,
                x: 2,
                y: 3,
                ..
            }) if message == "hi"
        ));
    }

    #[test]
    fn execute_task_unimplemented_returns_ok() {
        let (mut world, entity) = setup_inhabitant(2, 3, RelativeOrientation::Forward, 10, 10);
        let before = (
            world.get_component::<Position>(entity).unwrap().x,
            world.get_component::<Position>(entity).unwrap().y,
            *world.get_component::<RelativeOrientation>(entity).unwrap(),
        );

        let (response, _) = execute_task(&mut world, entity, &TaskType::Inventory);
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
        let mut world = World::default();
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
        let mut world = World::default();
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
        any_finished_task(&mut world);
        {
            let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
            assert_eq!(task_list.vector.len(), 1);
            task_list.vector[0].finish_on = Date::now().to_timestamp() - 1;
        }

        // task should be finished
        any_finished_task(&mut world);
        {
            let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
            assert_eq!(task_list.vector.len(), 0);
        }
    }
}
