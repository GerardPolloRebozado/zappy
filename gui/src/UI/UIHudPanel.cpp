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

    _bounds.Draw(raylib::Color(DARKGRAY).Fade(0.85f));
    _bounds.DrawLines(GOLD, 2.0f);

    raylib::Text infoText("Tile [" + std::to_string(selX) + ", " + std::to_string(selZ) + "]", 20,
                          GOLD, GetFontDefault(), 1.5f);
    infoText.Draw(_bounds.x + 15, _bounds.y + 15);

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
        auto drawResource = [&](const std::string& name, int count, raylib::Color col) {
            std::string text = name + ": " + std::to_string(count);
            raylib::Text(text, 16, col, GetFontDefault(), 1.5f)
                .Draw(_bounds.x + 20, (float)yOffset);
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
    raylib::Text("Entities:", 18, GOLD, GetFontDefault(), 1.5f)
        .Draw(_bounds.x + 15, (float)yOffset);
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
                raylib::Text(pInfo, 16, RED, GetFontDefault(), 1.5f)
                    .Draw(_bounds.x + 20, (float)yOffset);
                yOffset += 18;
                if (team) {
                    raylib::Text(team->team_name, 14, RAYWHITE, GetFontDefault(), 1.5f)
                        .Draw(_bounds.x + 35, (float)yOffset);
                    yOffset += 16;
                }
            }
        }
    }

    if (!foundPlayer) {
        raylib::Text("None", 16, GRAY, GetFontDefault(), 1.5f).Draw(_bounds.x + 20, (float)yOffset);
    }
}

} // namespace zappy
