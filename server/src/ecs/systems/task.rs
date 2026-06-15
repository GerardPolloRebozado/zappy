//! Timed execution of queued AI player commands.
//!
//! Commands accepted by [`crate::commands`] are not applied immediately.
//! [`crate::commands::queue_task`] appends a [`crate::ecs::components::task::Task`] to the inhabitant's
//! [`TaskList`] (FIFO, max 10). Each server tick, [`crate::ecs::systems::run::run_systems`]
//! calls [`any_finished_task`] to advance those queues.
//!
//! # Lifecycle
//!
//! Only the head of each [`TaskList`] runs at a time. A newly queued task has
//! [`TASK_NOT_STARTED`]; the next pass sets `finish_on` from [`TaskType::duration`]
//! and world frequency. When the timer elapses, the task is popped, its handler runs,
//! and the next queued task (if any) starts its timer.
//!
//! # Responses and side effects
//!
//! Most tasks push an `ok` [`Response`] onto the acting client's
//! [`NetworkData::pending_responses`]. Some also attach data (`Look`, `Inventory`) or
//! emit a [`ServerEvent`] for other clients (`BroadcastText`). [`TaskType::Death`] is
//! special: it writes `dead` straight to the socket and despawns the entity.
//!
//! Per-command logic lives in task-specific submodules: [`forward`], [`turn`],
//! [`look`], [`inventory`], [`broadcast`], [`death`], [`fork`], [`eject`], [`take_set`], and [`incantation`].
//!
//! # Broadcast flow
//!
//! 1. [`crate::commands`] queues [`TaskType::BroadcastText`] with the message text.
//! 2. When the timer elapses, [`broadcast::execute_broadcast`] returns `ok` plus a [`ServerEvent::Message`].
//! 3. [`any_finished_task`] pushes `ok` to the acting client, then calls
//!    [`broadcast_event`] so every AI receives `message k, text`
//!    and the GUI receives `pbc #n text` (see [`ServerEvent::to_ai_string`]).
//!
//! # Movement broadcast flow
//!
//! 1. [`crate::commands`] queues [`TaskType::Forward`], [`TaskType::TurnRight`],
//!    or [`TaskType::TurnLeft`].
//! 2. When the timer elapses, [`forward::execute_forward`] / [`turn::execute_turn_right`] /
//!    [`turn::execute_turn_left`] mutate [`crate::ecs::components::position::Position`] /
//!    [`crate::utils::orientation::RelativeOrientation`].
//! 3. Returns `ok` plus a [`ServerEvent::PlayerPosition`].
//! 4. [`broadcast_event`] pushes `ppo #n X Y O` to all GUI clients.

use crate::{
    ecs::{
        components::{
            inhabitant::Inhabitant,
            network::NetworkData,
            task::{TASK_NOT_STARTED, TaskList, TaskType},
            team::Team,
        },
        storage::{Entity, World},
    },
    protocol::{
        Response, ResponseCode,
        ServerEvent::{self},
        StatusCode::{self},
    },
    utils::date::Date,
};
use log::debug;

pub mod broadcast;
pub mod death;
pub mod eject;
pub mod fork;
pub mod forward;
pub mod incantation;
pub mod inventory;
pub mod look;
pub mod take_set;
pub mod turn;

/// Advances queued tasks, applies effects when a timer elapses, starts the next
/// task's timer, and handles command replies and broadcast events.
pub fn any_finished_task(world: &mut World) {
    let mut completed: Vec<(Entity, TaskType)> = Vec::new();
    let mut started_incantations: Vec<Entity> = Vec::new();
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
                    Date::now().to_timestamp() + (first_task.task_type.duration() * 1000 / freq);

                if matches!(first_task.task_type, TaskType::Incantation) {
                    started_incantations.push(*entity);
                }
                continue;
            }

            if !first_task.is_finished() {
                continue;
            }

            completed.push((*entity, first_task.task_type.clone()));

            task_list.vector.remove(0);

            if let Some(new_first_task) = task_list.vector.first_mut() {
                new_first_task.finish_on = Date::now().to_timestamp()
                    + (new_first_task.task_type.duration() * 1000 / freq);
            }
        }
    }

    incantation::process_started_incantations(world, started_incantations);

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
        } else if let Some(network_data) = world.get_component_mut::<NetworkData>(entity) {
            network_data.pending_responses.push(response);
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
            debug!("Broadcasting to GUI {}: {}", entity, line);
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

/// Dispatches a completed [`TaskType`] to its handler and returns the acting client's
/// response plus an optional broadcast event.
fn execute_task(
    world: &mut World,
    entity: Entity,
    task_type: &TaskType,
) -> (Response, Option<ServerEvent>) {
    match task_type {
        TaskType::Forward => forward::execute_forward(world, entity),
        TaskType::TurnRight => turn::execute_turn_right(world, entity),
        TaskType::TurnLeft => turn::execute_turn_left(world, entity),
        TaskType::Look => look::execute_look(world, entity),
        TaskType::Inventory => inventory::execute_inventory(world, entity),
        TaskType::Death => death::execute_death(),
        TaskType::BroadcastText(text) => broadcast::execute_broadcast(world, entity, text),
        TaskType::Take(resource) => take_set::take_task(world, entity, resource),
        TaskType::Set(resource) => take_set::set_task(world, entity, resource),
        TaskType::Incantation => incantation::execute_incantation(world, entity),
        TaskType::Fork => fork::execute_fork(),
        TaskType::Eject => eject::eject(world, entity),
    }
}

#[cfg(test)]
mod tests {
    use crate::ecs::components::task::{Task, TaskType};
    use crate::ecs::storage::World;

    use super::*;

    #[test]
    fn add_task() {
        let mut world = World::default();
        let entity = world.spawn();
        world.add_component(entity, TaskList::default());

        let task_list = world.get_component_mut::<TaskList>(entity).unwrap();
        let task = Task {
            task_type: TaskType::Forward,
            finish_on: TaskType::Forward.duration() * 1000 + Date::now().to_timestamp(),
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
                finish_on: TaskType::Forward.duration() * 1000 + Date::now().to_timestamp(),
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
