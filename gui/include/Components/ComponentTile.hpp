/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentTile.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTTILE_HPP
#define ZAPPY_GUI_COMPONENTTILE_HPP

struct TerrainModifiers {
    int visibility;
    double speedMul;
    double resourceMul;
    double soundMul;
};

struct Position {
    int x;
    int y;
};

struct Terrain {
    enum Type { PLAIN, MOUNTAIN, LAKE };
    Type current_type;
    int x;
    int y;
    TerrainModifiers modifiers;
    Position pos;
};


#endif //ZAPPY_GUI_COMPONENTTILE_HPP
