//! Map dimensions and protocol tile queries.
//!
//! Map bounds live on [`World::map_size`] so movement and `bct`/`mct` handlers
//! read the same values as the spawned tile grid.

use crate::ecs::storage::World;
use crate::game::Resource;

#[derive(Clone, Copy)]
pub struct MapSize {
    pub width: u32,
    pub height: u32,
}

/// Builds a `bct x y …` line for the tile at `(x, y)`, or `None` if no tile exists there.
pub fn get_tile_content(world: &World, x: u32, y: u32) -> Option<String> {
    use crate::ecs::components::inventory::Inventory;
    use crate::ecs::components::position::Position;
    use crate::ecs::components::terrain_type::TerrainType;
    use crate::ecs::components::tile::Tile;

    let tiles = world.get_storage::<Tile>()?;
    let positions = world.get_storage::<Position>()?;
    let inventories = world.get_storage::<Inventory>()?;
    let terrains = world.get_storage::<TerrainType>()?;

    for (ent, _tile) in tiles.iter() {
        let _pos = match positions.get(*ent) {
            Some(p) if p.x == x && p.y == y => p,
            _ => continue,
        };

        let inv = match inventories.get(*ent) {
            Some(i) => i,
            None => continue,
        };

        let terrain = match terrains.get(*ent) {
            Some(t) => t,
            None => continue,
        };

        let food = inv.items.get(&Resource::Food).unwrap_or(&0);
        let linemate = inv.items.get(&Resource::Linemate).unwrap_or(&0);
        let deraumere = inv.items.get(&Resource::Deraumere).unwrap_or(&0);
        let sibur = inv.items.get(&Resource::Sibur).unwrap_or(&0);
        let mendiane = inv.items.get(&Resource::Mendiane).unwrap_or(&0);
        let phiras = inv.items.get(&Resource::Phiras).unwrap_or(&0);
        let thystame = inv.items.get(&Resource::Thystame).unwrap_or(&0);

        let t_type = match terrain {
            TerrainType::Grass => 0,
            TerrainType::Mountain => 1,
            TerrainType::Water => 2,
            TerrainType::Sand => 3,
            TerrainType::Forest => 4,
            TerrainType::ObsidianBarrens => 5,
            TerrainType::LuminousOrchards => 6,
            TerrainType::CrystalCanyons => 7,
            TerrainType::MagneticTundra => 8,
        };

        return Some(format!(
            "bct {} {} {} {} {} {} {} {} {} {}",
            x, y, food, linemate, deraumere, sibur, mendiane, phiras, thystame, t_type
        ));
    }
    None
}
