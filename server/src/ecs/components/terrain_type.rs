/// Terrain type will be used to describe the terrain of each tile of the map
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TerrainType {
    // Standard Biomes
    Grass,
    Mountain,
    Water,
    Sand,
    Forest,

    // Special Biomes (Trantor)
    ObsidianBarrens,
    LuminousOrchards,
    CrystalCanyons,
    MagneticTundra,

    // Mechanics
    Wormhole,
}
