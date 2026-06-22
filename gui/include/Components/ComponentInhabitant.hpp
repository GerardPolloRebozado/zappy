/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentInhabitant.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTINHABITANT_HPP
#define ZAPPY_GUI_COMPONENTINHABITANT_HPP
#include "Color.hpp"
#include "ECS/World.hpp"
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

class TeamName {
  public:
    TeamName(std::string name, raylib::Color color) : _team_name(name), _color(color) {};
    raylib::Color _color;
    std::string _team_name;

    static raylib::Color findTeam(std::string name, World& w) {
        auto team = w.get_storage<TeamName>();
        if (team) {
            for (auto const& [entity, t] : *team) {
                if (t && t->_team_name == name) {
                    return t->_color;
                }
            }
        }
        return raylib::Color::White();
    }
};

struct Race {
    enum Type { ELF, DWARF, OGRE, TRANTORIAN };
    Type current_race;
};

struct Egg {
    int id;
};

struct RequestPlayerLevel {};
struct RequestPlayerInventory {};
} // namespace zappy
#endif // ZAPPY_GUI_COMPONENTINHABITANT_HPP
