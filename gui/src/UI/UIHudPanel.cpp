/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIHudPanel.cpp
*/

#include "UI/UIHudPanel.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
#include <raylib.h>
#include <string>

#include <limits>

namespace zappy {

UIHudPanel::UIHudPanel(raylib::Rectangle bounds, World& world, const RenderSystem& renderSystem,
                       int zIndex)
    : AUIComponent(bounds, zIndex), _world(world), _renderSystem(renderSystem) {}

void UIHudPanel::render() {
    auto [selX, selZ] = _renderSystem.getSelectedTile();

    if (selX == std::numeric_limits<int>::min()) {
        return;
    }

    DrawRectangleRec(_bounds, Fade(DARKGRAY, 0.85f));
    DrawRectangleLinesEx(_bounds, 2.0f, GOLD);

    std::string info = "Tile [" + std::to_string(selX) + ", " + std::to_string(selZ) + "]";
    ::DrawText(info.c_str(), _bounds.x + 15, _bounds.y + 15, 20, GOLD);

    auto terrainStorage = _world.get_storage<TerrainType>();
    std::shared_ptr<Inventory> inv = nullptr;

    if (terrainStorage) {
        for (auto const& [entity, type] : *terrainStorage) {
            auto pos = _world.get_component<Position>(entity);
            if (pos && pos->x == selX && pos->y == selZ) {
                inv = _world.get_component<Inventory>(entity);
                break;
            }
        }
    }

    int yOffset = _bounds.y + 50;
    if (inv) {
        auto drawResource = [&](const std::string& name, int count, Color col) {
            std::string text = name + ": " + std::to_string(count);
            ::DrawText(text.c_str(), _bounds.x + 20, yOffset, 16, col);
            yOffset += 20;
        };

        drawResource("Food", inv->food, ORANGE);
        drawResource("Linemate", inv->linemate, GREEN);
        drawResource("Deraumere", inv->deraumere, BLUE);
        drawResource("Sibur", inv->sibur, PURPLE);
        drawResource("Mendiane", inv->mendiane, YELLOW);
        drawResource("Phiras", inv->phiras, RED);
        drawResource("Thystame", inv->thystame, WHITE);
    }

    yOffset += 10;
    ::DrawText("Entities:", _bounds.x + 15, yOffset, 18, GOLD);
    yOffset += 25;

    bool foundPlayer = false;
    auto posStorage = _world.get_storage<Position>();
    if (posStorage) {
        for (auto const& [entity, pos] : *posStorage) {
            if (pos && pos->x == selX && pos->y == selZ) {
                auto level = _world.get_component<Level>(entity);
                auto team = _world.get_component<TeamName>(entity);

                if (!level && !team) {
                    continue; // Skip non-inhabitants
                }

                foundPlayer = true;
                std::string pInfo = "Player " + std::to_string(entity.id());
                if (level) {
                    pInfo += " (Lvl " + std::to_string(level->level) + ")";
                }
                ::DrawText(pInfo.c_str(), _bounds.x + 20, yOffset, 16, RED);
                yOffset += 18;
                if (team) {
                    ::DrawText(team->team_name.c_str(), _bounds.x + 35, yOffset, 14, RAYWHITE);
                    yOffset += 16;
                }
            }
        }
    }

    if (!foundPlayer) {
        ::DrawText("None", _bounds.x + 20, yOffset, 16, GRAY);
    }
}

} // namespace zappy