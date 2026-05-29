/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** RenderSystem.hpp
*/
#ifndef ZAPPY_GUI_RENDERSYSTEM_HPP
#define ZAPPY_GUI_RENDERSYSTEM_HPP
#include "../ECS/Register.hpp"

namespace zappy {
    class RenderSystem {
    public:
        void update(Register& r)
        {
            for (auto [entity, bot] : r._bots) {
                if (r._positions.find(entity) != r._positions.end()) {
                    auto pos = r._positions[entity];
                }
            }
        }
    };
} // zappy

#endif //ZAPPY_GUI_RENDERSYSTEM_HPP
