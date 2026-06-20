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
struct AnimatedResource {
    int resourceId = 0;
};
struct EventIncantationStart {
    std::vector<Entity> participants;
};
struct EventIncantationEnd {
    int result; // 1 for success, 0 for failure
};
struct EventDeath {};
struct TombTag {};
} // namespace zappy

#endif // ZAPPY_GUI_COMPONENTTAGS_HPP
