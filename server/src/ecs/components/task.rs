//! Timed player actions queued per inhabitant entity.
//!
//! Most variants are unit-like; [`TaskType::BroadcastText`] carries the message
//! string until the 7/f action completes and [`crate::ecs::systems::task`] builds
//! a [`crate::protocol::ServerEvent::Message`] for fan-out.

use crate::ecs::components::resource::Resource;

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
    Set(Resource),
    Incantation,
    Death,
}

impl std::fmt::Display for TaskType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            TaskType::Forward => write!(f, "Forward"),
            TaskType::TurnRight => write!(f, "TurnRight"),
            TaskType::TurnLeft => write!(f, "TurnLeft"),
            TaskType::Look => write!(f, "Look"),
            TaskType::Inventory => write!(f, "Inventory"),
            TaskType::BroadcastText(text) => write!(f, "BroadcastText({text})"),
            TaskType::Fork => write!(f, "Fork"),
            TaskType::Eject => write!(f, "Eject"),
            TaskType::Take(resource) => write!(f, "Take({resource})"),
            TaskType::Set(resource) => write!(f, "Set({resource})"),
            TaskType::Incantation => write!(f, "Incantation"),
            TaskType::Death => write!(f, "Death"),
        }
    }
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
            TaskType::Set(_) => 7,
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
    pub fn is_finished(&self, now: u64) -> bool {
        self.finish_on != TASK_NOT_STARTED && self.finish_on <= now
    }
}

#[derive(Default, Clone)]
pub struct TaskList {
    pub vector: Vec<Task>,
}
