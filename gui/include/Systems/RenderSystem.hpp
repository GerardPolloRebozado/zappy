/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** RenderSystem.hpp
*/
#ifndef ZAPPY_GUI_RENDERSYSTEM_HPP
#define ZAPPY_GUI_RENDERSYSTEM_HPP

#include "../ECS/Register.hpp"
#include <raylib.h>

namespace zappy {
    class RenderSystem {
    public:
        void update(Register& r) {
            for (auto& [entity, bot] : r._bots) {
                if (r._positions.find(entity) != r._positions.end()) {
                    auto& pos = r._positions[entity];
                    DrawCircle(pos.x, pos.y, 20.0f, BLUE);
                    DrawText(bot.team_name.c_str(), pos.x - 15, pos.y - 30, 10, WHITE);
                }
            }
        }
    };
}

#endif