use crate::ecs::components::inventory::Inventory;
use crate::game::Resource;

pub fn add_item(inventory: &mut Inventory, resource: Resource, amount: u32) {
    *inventory.items.entry(resource).or_insert(0) += amount;
}

pub fn remove_item(inventory: &mut Inventory, resource: Resource, amount: u32) -> bool {
    let entry = inventory.items.entry(resource).or_insert(0);
    if *entry >= amount {
        *entry -= amount;
        true
    } else {
        false
    }
}

pub fn get_item_count(inventory: &Inventory, resource: Resource) -> u32 {
    *inventory.items.get(&resource).unwrap_or(&0)
}

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
