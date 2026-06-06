//! Timed player actions queued per inhabitant entity.
//!
//! Most variants are unit-like; [`TaskType::BroadcastText`] carries the message
//! string until the 7/f action completes and [`crate::ecs::systems::task`] builds
//! a [`crate::protocol::ServerEvent::Message`] for fan-out.

use crate::game::{Date, Resource};

#[derive(Clone)]
pub enum TaskType {
    Forward,
    TurnRight,
    TurnLeft,
    Look,
    Inventory,
    /// `Broadcast <text>` — duration 7/f. The `String` is the text to relay after completion.
    BroadcastText(String),
    Fork,
    Eject,
    Take(Resource),
    ///< Get the resource type and the entity of the tile
    Drop,
    Incantation,
    Death,
}

impl TaskType {
    /// return the duration of the task in time units.  unit / freq = seconds
    pub fn duration(&self) -> u64 {
        match self {
            TaskType::Forward => 7,
            TaskType::TurnRight => 7,
            TaskType::TurnLeft => 7,
            TaskType::Look => 7,
            TaskType::Inventory => 1,
            TaskType::BroadcastText(_) => 7,
            TaskType::Fork => 42,
            TaskType::Eject => 7,
            TaskType::Take(_) => 7,
            TaskType::Drop => 7,
            TaskType::Incantation => 300,
            TaskType::Death => 0,
        }
    }
}

pub const TASK_NOT_STARTED: u64 = 0;

#[derive(Clone)]
/// Task type and whenn will it finish if started now, if finish_on = 0 then the task is not started yet
pub struct Task {
    pub task_type: TaskType,
    pub finish_on: u64,
}

impl Task {
    pub fn is_finished(&self) -> bool {
        self.finish_on != TASK_NOT_STARTED && self.finish_on <= Date::now().to_timestamp()
    }
}

#[derive(Default, Clone)]
pub struct TaskList {
    pub vector: Vec<Task>,
}
