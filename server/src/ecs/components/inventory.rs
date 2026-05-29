use crate::game::Resource;
use std::collections::HashMap;

/// Pure data component for storing items
#[derive(Debug, Default, Clone)]
pub struct Inventory {
    pub items: HashMap<Resource, u32>,
}
