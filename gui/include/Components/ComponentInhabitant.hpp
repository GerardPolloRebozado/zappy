/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentInhabitant.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTINHABITANT_HPP
#define ZAPPY_GUI_COMPONENTINHABITANT_HPP
#include <string>
namespace zappy {

struct Level {
    int level;
};

struct Orientation {
    enum Direction { N = 1, E = 2, S = 3, W = 4 };
    Direction current_direction;
};

struct VisionRange {
    int vision_range;
};

struct TeamName {
    std::string team_name;
};

struct Race {
    enum Type { ELF, DWARF, OGRE, TRANTORIAN };
    Type current_race;
};

struct Egg {
    int id;
};
} // namespace zappy
#endif // ZAPPY_GUI_COMPONENTINHABITANT_HPP
