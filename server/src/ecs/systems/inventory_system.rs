//! # Inventory System
//!
//! This module provides functions to manipulate `Inventory` components.
//! Following strict ECS principles, these functions are stateless and operate
//! directly on the component data.

use crate::ecs::components::inventory::Inventory;
use crate::game::Resource;

/// Adds a specified amount of a resource to an inventory.
pub fn add_item(inventory: &mut Inventory, resource: Resource, amount: u32) {
    *inventory.items.entry(resource).or_insert(0) += amount;
}

/// Removes a specified amount of a resource from an inventory.
///
/// Returns `true` if the items were successfully removed, or `false` if the
/// inventory did not contain enough of the resource.
pub fn remove_item(inventory: &mut Inventory, resource: Resource, amount: u32) -> bool {
    let entry = inventory.items.entry(resource).or_insert(0);
    if *entry >= amount {
        *entry -= amount;
        true
    } else {
        false
    }
}

/// Returns the current count of a specific resource in an inventory.
pub fn get_item_count(inventory: &Inventory, resource: Resource) -> u32 {
    *inventory.items.get(&resource).unwrap_or(&0)
}

/// Formats the inventory contents into a space-separated string of resource names.
///
/// The resources are ordered according to the project specification:
/// food, linemate, deraumere, sibur, mendiane, phiras, thystame.
pub fn format_inventory(inventory: &Inventory) -> String {
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
        let count = get_item_count(inventory, resource);
        for _ in 0..count {
            names.push(resource.to_string());
        }
    }
    names.join(" ")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_inventory_ops() {
        let mut inv = Inventory::default();

        add_item(&mut inv, Resource::Food, 2);
        assert_eq!(get_item_count(&inv, Resource::Food), 2);

        assert!(remove_item(&mut inv, Resource::Food, 1));
        assert_eq!(get_item_count(&inv, Resource::Food), 1);

        assert!(!remove_item(&mut inv, Resource::Food, 5));
        assert_eq!(get_item_count(&inv, Resource::Food), 1);
    }

    #[test]
    fn test_format_inventory() {
        let mut inv = Inventory::default();
        add_item(&mut inv, Resource::Food, 2);
        add_item(&mut inv, Resource::Linemate, 1);

        let s = format_inventory(&inv);
        assert_eq!(s, "food food linemate");
    }
}
