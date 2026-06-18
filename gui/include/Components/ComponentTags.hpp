/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentTags.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTTAGS_HPP
#define ZAPPY_GUI_COMPONENTTAGS_HPP

#include "ECS/Entity.hpp"
#include <vector>

namespace zappy {
struct TileTag {};
struct InhabitantTag {};
struct EggTag {};
struct MapTag {};
struct TeamTag {};
struct EventEggHatched {};

struct EventIncantationStart {
    std::vector<Entity> participants;
};

struct EventIncantationEnd {
    int result; // 1 for success, 0 for failure
};
} // namespace zappy

#endif // ZAPPY_GUI_COMPONENTTAGS_HPP
