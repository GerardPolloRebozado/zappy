/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentBot.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTBOT_HPP
#define ZAPPY_GUI_COMPONENTBOT_HPP
#include <string>
namespace zappy {
    struct InhabitantData {
        int id;
        int level;
        int orientation;
        std::string team_name;
        int vision_range;
    };

    struct Race {
        enum Type { ELF, DWARF, OGRE, TRANTORIAN };
        Type current_race;
    };
}
#endif //ZAPPY_GUI_COMPONENTBOT_HPP
