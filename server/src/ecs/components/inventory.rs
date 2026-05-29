use crate::game::Resource;
use std::collections::HashMap;

#[derive(Debug, Default)]
pub struct Inventory {
    pub items: HashMap<Resource, u32>,
}

impl Inventory {
    pub fn new() -> Self {
        Self {
            items: HashMap::new(),
        }
    }

    pub fn add(&mut self, resource: Resource, amount: u32) {
        *self.items.entry(resource).or_insert(0) += amount;
    }

    pub fn remove(&mut self, resource: Resource, amount: u32) -> bool {
        let entry = self.items.entry(resource).or_insert(0);
        if *entry >= amount {
            *entry -= amount;
            true
        } else {
            false
        }
    }

    pub fn get(&self, resource: Resource) -> u32 {
        *self.items.get(&resource).unwrap_or(&0)
    }

    pub fn to_protocol_string(&self) -> String {
        let mut names = Vec::new();
        let resource_order = [
            Resource::Food,
            Resource::Linemate,
            Resource::Deraumere,
            Resource::Sibur,
            Resource::Mendiane,
            Resource::Phiras,
            Resource::Thystame,
        ];

        for resource in resource_order {
            let count = self.get(resource);
            for _ in 0..count {
                names.push(resource.to_string());
            }
        }
        names.join(" ")
    }
}
