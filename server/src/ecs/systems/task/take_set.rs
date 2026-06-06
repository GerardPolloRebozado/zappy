use log::error;

use crate::{
    ecs::{
        components::{inventory::Inventory, position::Position, tile::Tile},
        storage::{Entity, World},
    },
    game::Resource,
    protocol::{
        Response, ResponseCode,
        ServerEvent::{self, ResourceCollect},
        StatusCode,
    },
};

pub fn take_task(
    world: &mut World,
    entity: Entity,
    resource: &Resource,
) -> (Response, Option<ServerEvent>) {
    let ok = Response::new(ResponseCode::Status(StatusCode::Ok), None);

    let player_position = world.get_component::<Position>(entity).unwrap().clone();
    let tile_entity = Tile::find_tile_by_pos(&player_position, world);
    if tile_entity.is_none() {
        error!(
            "Could not find tile at position ({}, {})",
            player_position.x, player_position.y
        );
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let tile_entity = tile_entity.unwrap();
    let tile_component = world.get_component_mut::<Inventory>(tile_entity);
    if tile_component.is_none() {
        error!("Could not get inventory of tile {}", tile_entity);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let tile_component = tile_component.unwrap();
    let extracted_resource = tile_component.remove_item(*resource);
    if !extracted_resource {
        error!("Could not get resource {}", resource);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let player_inventory = world.get_component_mut::<Inventory>(entity);
    if player_inventory.is_none() {
        error!("Could not get inventory of player {}", entity);
        return (
            Response::new(ResponseCode::Status(StatusCode::Ko), None),
            None,
        );
    }
    let player_invetory = player_inventory.unwrap();
    player_invetory.add_item(*resource);
    (
        ok,
        Some(ResourceCollect {
            player_id: entity.id(),
            resource: *resource as u8,
        }),
    )
}
