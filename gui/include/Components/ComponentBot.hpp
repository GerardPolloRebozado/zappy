/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentBot.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTBOT_HPP
#define ZAPPY_GUI_COMPONENTBOT_HPP
#include <string>

struct Race {
    enum Type { ELF, DWARF, OGRE, TRANTORIAN };
    Type current_race;
};

struct Position3D {
    double x;
    double y;
    double z;
};

struct ComponentBot {
    int id;
    int level;
    Position3D pos3d;
    std::string team;
    int orientation;
    int vision_range;
    Race race;
};

#endif //ZAPPY_GUI_COMPONENTBOT_HPP
