/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AssetManager.hpp
*/
#ifndef ZAPPY_GUI_ASSETMANAGER_HPP
#define ZAPPY_GUI_ASSETMANAGER_HPP
#include "raylib-cpp.hpp"
namespace zappy {

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

} // zappy

#endif //ZAPPY_GUI_ASSETMANAGER_HPP
