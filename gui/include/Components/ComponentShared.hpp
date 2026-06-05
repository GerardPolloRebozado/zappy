/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentShared.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTSHARED_HPP
#define ZAPPY_GUI_COMPONENTSHARED_HPP
#include "raylib-cpp.hpp"

namespace zappy {
struct Position {
    int x;
    int y;
};

struct Size {
    int width;
    int height;
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

struct TimeUnit {
    int frequency;
};
} // namespace zappy

#endif // ZAPPY_GUI_COMPONENTSHARED_HPP
