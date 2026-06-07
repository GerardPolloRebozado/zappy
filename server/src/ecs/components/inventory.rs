use crate::game::Resource;
use std::collections::HashMap;

/// Pure data component for storing items
#[derive(Debug, Default, Clone)]
pub struct Inventory {
    pub items: HashMap<Resource, u32>,
}

impl Inventory {
    pub fn new() -> Self {
        Inventory {
            items: HashMap::new(),
        }
    }
    pub fn add_item(&mut self, resource: Resource) {
        let quantity = self.items.entry(resource).or_insert(0);
        *quantity += 1;
    }

    pub fn remove_item(&mut self, resource: Resource) -> bool {
        let quantity = self.items.get_mut(&resource);
        if let Some(quantity) = quantity {
            *quantity -= 1;
            if *quantity == 0 {
                self.items.remove(&resource);
            }
            true
        } else {
            false
        }
    }
}
