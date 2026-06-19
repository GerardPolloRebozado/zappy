/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ComponentIncantationEffect.hpp
*/
#ifndef ZAPPY_COMPONENTINCANTATIONEFFECT_HPP
#define ZAPPY_COMPONENTINCANTATIONEFFECT_HPP

#include "ECS/Entity.hpp"
#include <vector>

namespace zappy {

/**
 * @struct ComponentIncantationEffect
 * @brief Tag component for tracking active incantation particle emitters
 */
struct ComponentIncantationEffect {
    int x;
    int y;
    std::vector<Entity> participants;
    float timeElapsed = 0.0f;
};

} // namespace zappy

#endif // ZAPPY_COMPONENTINCANTATIONEFFECT_HPP
