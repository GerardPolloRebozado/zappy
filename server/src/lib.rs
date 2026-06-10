#![doc = include_str!("../README.md")]

pub mod commands;
pub mod ecs;
pub mod protocol;
pub mod server;
pub mod utils;

pub use ecs::components::inhabitant::Inhabitant;
