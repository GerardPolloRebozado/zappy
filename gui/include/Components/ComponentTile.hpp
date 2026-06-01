/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentTile.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTTILE_HPP
#define ZAPPY_GUI_COMPONENTTILE_HPP
namespace zappy {
struct TerrainModifiers {
    int visibility;
    double speedMul;
    double resourceMul;
    double soundMul;
};

struct TerrainType {
    enum Type {
        // Standard
        GRASS,
        MOUNTAIN,
        WATER,
        SAND,
        FOREST,
        // Trantor
        OBSIDIAN_BARRENS,
        LUMINOUS_ORCHARDS,
        CRYSTAL_CANYONS,
        MAGNETIC_TUNDRA
    };
    Type current_type;
};
} // namespace zappy
#endif // ZAPPY_GUI_COMPONENTTILE_HPP
