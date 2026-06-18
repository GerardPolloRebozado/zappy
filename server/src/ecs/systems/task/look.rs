use crate::{
    ecs::{
        components::{inventory::Inventory, level::Level, position::Position, tile::Tile},
        storage::{Entity, World},
    },
    protocol::{Response, ResponseCode, ServerEvent, StatusCode},
    utils::orientation::RelativeOrientation,
};

/// Runs the AI `Look` task for `entity`.
///
/// Reads the surrounding tiles from the entity's [`Position`], [`RelativeOrientation`],
/// and [`Level`], and returns `ok` with a bracket-formatted vision string. Does not mutate the world.
pub fn execute_look(world: &World, entity: Entity) -> (Response, Option<ServerEvent>) {
    (
        Response::new(
            ResponseCode::Status(StatusCode::Ok),
            Some(format_look_response(world, entity)),
        ),
        None,
    )
}

fn format_look_response(world: &World, entity: Entity) -> String {
    let pos = match world.get_component::<Position>(entity) {
        Some(p) => p,
        None => return "[]".to_string(),
    };
    let ori = match world.get_component::<RelativeOrientation>(entity) {
        Some(o) => *o,
        None => return "[]".to_string(),
    };
    let level = world
        .get_component::<Level>(entity)
        .map(|l| l.value)
        .unwrap_or(1);

    // TODO: Implement vision modifications from biomes (e.g. Luminous Orchards)
    // TODO: Implement Solar Flare global vision
    let tiles_content: Vec<String> = (0..=i32::from(level))
        .flat_map(|i| (-i..=i).map(move |j| (i, j)))
        .map(|(f, r)| {
            let (x, y) =
                get_relative_coords(pos, ori, f, r, world.map_size.width, world.map_size.height);
            get_tile_info(world, x, y)
        })
        .collect();

    format!("[{}]", tiles_content.join(","))
}

fn get_relative_coords(
    pos: &Position,
    ori: RelativeOrientation,
    f: i32,
    r: i32,
    width: u32,
    height: u32,
) -> (u32, u32) {
    let (ax, ay) = match ori {
        RelativeOrientation::Forward => (
            i32::try_from(pos.x).unwrap() + r,
            i32::try_from(pos.y).unwrap() - f,
        ),
        RelativeOrientation::Right => (
            i32::try_from(pos.x).unwrap() + f,
            i32::try_from(pos.y).unwrap() + r,
        ),
        RelativeOrientation::Back => (
            i32::try_from(pos.x).unwrap() - r,
            i32::try_from(pos.y).unwrap() + f,
        ),
        RelativeOrientation::Left => (
            i32::try_from(pos.x).unwrap() - f,
            i32::try_from(pos.y).unwrap() - r,
        ),
        _ => (i32::try_from(pos.x).unwrap(), i32::try_from(pos.y).unwrap()),
    };

    let x = u32::try_from(ax.rem_euclid(i32::try_from(width).unwrap())).unwrap();
    let y = u32::try_from(ay.rem_euclid(i32::try_from(height).unwrap())).unwrap();
    (x, y)
}

fn get_tile_info(world: &World, x: u32, y: u32) -> String {
    let mut info = Vec::new();

    info.extend(get_players_on_tile(world, x, y));
    info.extend(get_resources_on_tile(world, x, y));

    info.join(" ")
}

fn get_players_on_tile(world: &World, x: u32, y: u32) -> Vec<String> {
    let (Some(positions), Some(orientations)) = (
        world.get_storage::<Position>(),
        world.get_storage::<RelativeOrientation>(),
    ) else {
        return Vec::new();
    };

    positions
        .iter()
        .filter(|&(ent, pos)| pos.x == x && pos.y == y && orientations.get(*ent).is_some())
        .map(|_| "player".to_string())
        .collect()
}

fn get_resources_on_tile(world: &World, x: u32, y: u32) -> Vec<String> {
    let (Some(tiles), Some(positions), Some(inventories)) = (
        world.get_storage::<Tile>(),
        world.get_storage::<Position>(),
        world.get_storage::<Inventory>(),
    ) else {
        return Vec::new();
    };

    for (ent, _tile) in tiles.iter() {
        let (Some(pos), Some(inv)) = (positions.get(*ent), inventories.get(*ent)) else {
            continue;
        };

        if pos.x == x && pos.y == y {
            let mut resources: Vec<String> = inv
                .items
                .iter()
                .flat_map(|(res, count)| std::iter::repeat_n(res.to_string(), *count as usize))
                .collect();
            resources.sort();
            return resources;
        }
    }
    Vec::new()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ecs::components::level::Level;
    use crate::ecs::components::resource::Resource;

    #[test]
    fn test_execute_look() {
        let mut world = World::default();
        world.map_size.width = 10;
        world.map_size.height = 10;

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

        let (response, event) = execute_look(&world, entity);
        // Level 1 vision: 4 tiles.
        // Tile 0: (5,5) -> "player"
        // Tile 1: (4,4) -> ""
        // Tile 2: (5,4) -> "food"
        // Tile 3: (6,4) -> ""
        // Note: players on the same tile are also listed. Our player is at (5,5).
        assert_eq!(response.data.as_deref(), Some("[player,,food,]"));
        assert!(event.is_none());
    }
}
