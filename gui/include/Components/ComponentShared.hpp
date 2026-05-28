/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentShared.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTSHARED_HPP
#define ZAPPY_GUI_COMPONENTSHARED_HPP
#include "raylib-cpp.hpp"

struct Position {
    int x;
    int y;
};

struct Inventory {
    int food;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
};

struct Renderable3D {
    std::string model_id;
    raylib::Color color;
};

struct Assets {
    //stones
    raylib::Model Linemate;
    raylib::Model Deraumere;
    raylib::Model Sibur;
    raylib::Model Mendiane;
    raylib::Model Phiras;
    raylib::Model Thystame;
    // foods
    raylib::Model Food;
    //bots
    raylib::Model TrantorianNico;
    //tiles
    raylib::Model TileMountain;
    raylib::Model TileLake;
    raylib::Model TilePlain;
};



#endif //ZAPPY_GUI_COMPONENTSHARED_HPP
