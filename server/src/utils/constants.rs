pub const MAX_NAME_LENGTH: usize = 32;
pub const MAX_DESCRIPTION_LENGTH: usize = 255;
pub const MAX_BODY_LENGTH: usize = 512;
pub const STARTING_LIFE_UNITS: u64 = 10;
pub const LIFE_UNIT_IN_TIME_UNITS: u64 = 126;
pub const ERROR_EXIT_CODE: i32 = 84;
/// time units that need to pass to spawn new resources. to get seconds: time units / frequency = time in seconds
pub const UNITS_BETWEEN_RESOURCE_SPAWN: u64 = 20;
/// resources density
pub const FOOD_DENSITY: f32 = 0.5;
pub const LINEMATE_DENSITY: f32 = 0.3;
pub const DERAUMERE_DENSITY: f32 = 0.15;
pub const SIBUR_DENSITY: f32 = 0.1;
pub const MENDIANE_DENSITY: f32 = 0.1;
pub const PHIRAS_DENSITY: f32 = 0.08;
pub const THYSTAME_DENSITY: f32 = 0.05;
