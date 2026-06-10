use crate::ecs::components::resource::Resource;
use std::collections::HashMap;

/// Pure data component for storing items
#[derive(Debug, Default, Clone, PartialEq, Eq)]
pub struct Inventory {
    pub items: HashMap<Resource, u32>,
}

const RESOURCE_ORDER: [Resource; 7] = [
    Resource::Food,
    Resource::Linemate,
    Resource::Deraumere,
    Resource::Sibur,
    Resource::Mendiane,
    Resource::Phiras,
    Resource::Thystame,
];

impl Inventory {
    pub fn new() -> Self {
        Inventory {
            items: HashMap::new(),
        }
    }

    /// Adds a specified amount of a resource to an inventory.
    pub fn add_item(&mut self, resource: Resource, amount: u32) {
        *self.items.entry(resource).or_insert(0) += amount;
    }

    /// Removes a specified amount of a resource from an inventory.
    ///
    /// Returns `true` if the items were successfully removed, or `false` if the
    /// inventory did not contain enough of the resource.
    pub fn remove_item(&mut self, resource: Resource, amount: u32) -> bool {
        let entry = self.items.entry(resource).or_insert(0);
        if *entry >= amount {
            *entry -= amount;
            true
        } else {
            false
        }
    }

    /// Returns the current count of a specific resource in an inventory.
    pub fn get_item_count(&self, resource: Resource) -> u32 {
        *self.items.get(&resource).unwrap_or(&0)
    }

    /// Formats the inventory contents into a space-separated string of resource names.
    ///
    /// The resources are ordered according to the project specification:
    /// food, linemate, deraumere, sibur, mendiane, phiras, thystame.
    pub fn format_inventory(&self) -> String {
        let mut names = Vec::new();

        for resource in RESOURCE_ORDER {
            let count = self.get_item_count(resource);
            for _ in 0..count {
                names.push(resource.to_string());
            }
        }
        names.join(" ")
    }

    /// Formats the inventory for the AI `Inventory` command response.
    ///
    /// Always includes all seven resources in specification order, e.g.
    /// `[food 10, linemate 5, deraumere 0, sibur 1, mendiane 2, phiras 3, thystame 4]`.
    pub fn format_inventory_response(&self) -> String {
        let parts: Vec<String> = RESOURCE_ORDER
            .iter()
            .map(|&resource| format!("{} {}", resource, self.get_item_count(resource)))
            .collect();
        format!("[{}]", parts.join(", "))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_inventory_ops() {
        let mut inv = Inventory::default();

        inv.add_item(Resource::Food, 2);
        assert_eq!(inv.get_item_count(Resource::Food), 2);

        assert!(inv.remove_item(Resource::Food, 1));
        assert_eq!(inv.get_item_count(Resource::Food), 1);

        assert!(!inv.remove_item(Resource::Food, 5));
        assert_eq!(inv.get_item_count(Resource::Food), 1);
    }

    #[test]
    fn test_format_inventory() {
        let mut inv = Inventory::default();
        inv.add_item(Resource::Food, 2);
        inv.add_item(Resource::Linemate, 1);

        let s = inv.format_inventory();
        assert_eq!(s, "food food linemate");
    }

    #[test]
    fn test_format_inventory_response_empty() {
        let inv = Inventory::default();
        assert_eq!(
            inv.format_inventory_response(),
            "[food 0, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0]"
        );
    }

    #[test]
    fn test_format_inventory_response_mixed() {
        let mut inv = Inventory::default();
        inv.add_item(Resource::Food, 10);
        inv.add_item(Resource::Linemate, 5);
        inv.add_item(Resource::Sibur, 1);
        inv.add_item(Resource::Mendiane, 2);
        inv.add_item(Resource::Phiras, 3);
        inv.add_item(Resource::Thystame, 4);

        assert_eq!(
            inv.format_inventory_response(),
            "[food 10, linemate 5, deraumere 0, sibur 1, mendiane 2, phiras 3, thystame 4]"
        );
    }
}
