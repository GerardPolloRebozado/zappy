use crate::{
    ecs::{
        components::{
            inventory::Inventory,
            level::Level,
            position::Position,
            tile::Tile,
        },
        storage::{Entity, World},
    },
    utils::orientation::RelativeOrientation,
};

pub fn execute_look(world: &World, entity: Entity) -> String {
    let pos = match world.get_component::<Position>(entity) {
        Some(p) => p,
        None => return "[]".to_string(),
    };
    let ori = match world.get_component::<RelativeOrientation>(entity) {
        Some(o) => o,
        None => return "[]".to_string(),
    };
    let level = world
        .get_component::<Level>(entity)
        .map(|l| l.value)
        .unwrap_or(1);
    let map_width = world.mapSize.width;
    let map_height = world.mapSize.height;

    let mut tiles_content = Vec::new();

    // TODO: Implement vision modifications from biomes (e.g. Luminous Orchards)
    // TODO: Implement Solar Flare global vision
    for i in 0..=i32::from(level) {
        for j in -i..=i {
            let (f, r) = (i, j);
            let (ax, ay) = match ori {
                RelativeOrientation::Forward => (
                    i32::try_from(pos.x).unwrap() + r,
                    i32::try_from(pos.y).unwrap() - f,
                ),
                RelativeOrientation::ForwardLeft => (
                    i32::try_from(pos.x).unwrap() + f,
                    i32::try_from(pos.y).unwrap() + r,
                ),
                RelativeOrientation::Left => (
                    i32::try_from(pos.x).unwrap() - r,
                    i32::try_from(pos.y).unwrap() + f,
                ),
                RelativeOrientation::BackLeft => (
                    i32::try_from(pos.x).unwrap() - f,
                    i32::try_from(pos.y).unwrap() - r,
                ),
                _ => (i32::try_from(pos.x).unwrap(), i32::try_from(pos.y).unwrap()),
            };
            let x = u32::try_from(ax.rem_euclid(i32::try_from(map_width).unwrap())).unwrap();
            let y = u32::try_from(ay.rem_euclid(i32::try_from(map_height).unwrap())).unwrap();

            tiles_content.push(get_tile_info(world, x, y));
        }
    }

    format!("[{}]", tiles_content.join(","))
}

fn get_tile_info(world: &World, x: u32, y: u32) -> String {
    let mut info = Vec::new();

    // 1. Players on this tile
    if let (Some(positions), Some(orientations)) = (
        world.get_storage::<Position>(),
        world.get_storage::<RelativeOrientation>(),
    ) {
        for (ent, pos) in positions.iter() {
            if pos.x == x && pos.y == y && orientations.get(*ent).is_some() {
                info.push("player".to_string());
            }
        }
    }

    // 2. Resources on this tile
    if let (Some(tiles), Some(positions), Some(inventories)) = (
        world.get_storage::<Tile>(),
        world.get_storage::<Position>(),
        world.get_storage::<Inventory>(),
    ) {
        for (ent, _tile) in tiles.iter() {
            if let Some(pos) = positions.get(*ent) {
                if pos.x == x && pos.y == y {
                    if let Some(inv) = inventories.get(*ent) {
                        let mut resources: Vec<String> = Vec::new();
                        for (res, count) in &inv.items {
                            for _ in 0..*count {
                                resources.push(res.to_string());
                            }
                        }
                        // Sort resources to have a consistent output (optional but good for tests)
                        resources.sort();
                        info.extend(resources);
                    }
                    break;
                }
            }
        }
    }

    info.join(" ")
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::components::level::Level;
    use crate::game::Resource;

    #[test]
    fn test_execute_look() {
        let mut world = World::new();
        world.mapSize.width = 10;
        world.mapSize.height = 10;
        world.register_component::<Position>();
        world.register_component::<RelativeOrientation>();
        world.register_component::<Level>();
        world.register_component::<Tile>();
        world.register_component::<Inventory>();

        let entity = world.spawn();
        world.add_component(entity, Position { x: 5, y: 5 });
        world.add_component(entity, RelativeOrientation::Forward);
        world.add_component(entity, Level { value: 1 });

        // Add a resource on tile (5, 4) - which is in front of (5, 5) facing North
        let tile = world.spawn();
        world.add_component(tile, Tile);
        world.add_component(tile, Position { x: 5, y: 4 });
        let mut inv = Inventory::new();
        inv.items.insert(Resource::Food, 1);
        world.add_component(tile, inv);

        let data = execute_look(&world, entity);
        // Level 1 vision: 4 tiles.
        // Tile 0: (5,5) -> "player"
        // Tile 1: (4,4) -> ""
        // Tile 2: (5,4) -> "food"
        // Tile 3: (6,4) -> ""
        // Note: players on the same tile are also listed. Our player is at (5,5).
        assert_eq!(data, "[player,,food,]");
    }
}
