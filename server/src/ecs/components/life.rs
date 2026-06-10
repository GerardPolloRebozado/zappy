use crate::utils::{
    constants::{LIFE_UNIT_IN_TIME_UNITS, STARTING_LIFE_UNITS},
    date::Date,
};

/// Used to store the time a entity will day in unix seconds
#[derive(Debug, Clone, PartialEq, Eq, Copy)]
pub struct Life {
    pub death_on: u64,
}

impl Life {
    pub fn new(freq: u64) -> Self {
        let curr_date = Date::now().to_timestamp();
        Self {
            death_on: curr_date + STARTING_LIFE_UNITS * LIFE_UNIT_IN_TIME_UNITS / freq,
        }
    }

    /// returns true if alive
    pub fn is_alive(&self) -> bool {
        let curr_date = Date::now().to_timestamp();
        curr_date < self.death_on
    }
}
