use crate::game::Date;

pub enum TaskType {
    Forward,
    TurnRight,
    TurnLeft,
    Look,
    Inventory,
    BroadcastText,
    Fork,
    Eject,
    Take,
    Drop,
    Incantation,
}

impl TaskType {
    pub fn duration(&self) -> u64 {
        match self {
            TaskType::Forward => 7,
            TaskType::TurnRight => 7,
            TaskType::TurnLeft => 7,
            TaskType::Look => 7,
            TaskType::Inventory => 1,
            TaskType::BroadcastText => 7,
            TaskType::Fork => 42,
            TaskType::Eject => 7,
            TaskType::Take => 7,
            TaskType::Drop => 7,
            TaskType::Incantation => 300,
        }
    }
}

pub struct Task {
    pub task_type: TaskType,
    pub finish_on: u64,
}

impl Task {
    pub fn is_finished(&self) -> bool {
        self.finish_on <= Date::now().to_timestamp()
    }
}

#[derive(Default)]
pub struct TaskList {
    pub vector: Vec<Task>,
}
