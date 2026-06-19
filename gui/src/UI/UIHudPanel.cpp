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
#include "Graphics/AssetManager.hpp"
#include <string>

#include <limits>

namespace zappy {

UIHudPanel::UIHudPanel(raylib::Rectangle bounds, World& world, const RenderSystem& renderSystem,
                       std::function<void()> onClick, int zIndex)
    : AUIComponent(bounds, onClick, zIndex), _world(world), _renderSystem(renderSystem) {}

void UIHudPanel::render() {
    auto [selX, selZ] = _renderSystem.getSelectedTile();

    if (selX == std::numeric_limits<int>::min()) {
        return;
    }

    auto& tex = AssetManager::getInstance().getTexture("menu_bg");
    if (tex.id != 0) {
        ::NPatchInfo npatchInfo = {
            (raylib::Rectangle){0.0f, 0.0f, (float)tex.width, (float)tex.height},
            20,
            20,
            20,
            20, // margin left, top, right, bottom
            NPATCH_NINE_PATCH};
        tex.Draw(npatchInfo, _bounds, raylib::Vector2(0, 0), 0.0f, raylib::Color::White());
    } else {
        _bounds.Draw(raylib::Color(15, 20, 40, 230));
        _bounds.DrawLines(raylib::Color(0, 100, 255, 255), 2.0f);
    }

    raylib::Text infoText("Tile [" + std::to_string(selX) + ", " + std::to_string(selZ) + "]", 20,
                          raylib::Color(80, 50, 40, 255),
                          AssetManager::getInstance().getFont("TextFont"), 1.5f);
    infoText.Draw(_bounds.x + 15, _bounds.y + 15);

    auto terrainStorage = _world.get_storage<TerrainType>();
    std::shared_ptr<Inventory> inv = nullptr;
    std::shared_ptr<TerrainType> terrain = nullptr;

    if (terrainStorage) {
        for (auto const& [entity, type] : *terrainStorage) {
            auto pos = _world.get_component<Position>(entity);
            if (pos && pos->x == selX && pos->y == selZ) {
                inv = _world.get_component<Inventory>(entity);
                terrain = _world.get_component<TerrainType>(entity);
                break;
            }
        }
    }

    int yOffset = _bounds.y + 45;

    if (terrain) {
        std::string biomeName = "Unknown";
        std::string biomeEffect = "";
        switch (terrain->current_type) {
            case TerrainType::GRASS:
                biomeName = "Grass";
                break;
            case TerrainType::MOUNTAIN:
                biomeName = "Mountain";
                break;
            case TerrainType::WATER:
                biomeName = "Water";
                break;
            case TerrainType::SAND:
                biomeName = "Sand";
                break;
            case TerrainType::FOREST:
                biomeName = "Forest";
                break;
            case TerrainType::OBSIDIAN_BARRENS:
                biomeName = "Obsidian Barrens";
                biomeEffect = "Food x0.2, Gems x1.5";
                break;
            case TerrainType::LUMINOUS_ORCHARDS:
                biomeName = "Luminous Orchards";
                biomeEffect = "Food x2.0, Vision -1";
                break;
            case TerrainType::CRYSTAL_CANYONS:
                biomeName = "Crystal Canyons";
                biomeEffect = "Gems x1.5, Bcast x1.5";
                break;
            case TerrainType::MAGNETIC_TUNDRA:
                biomeName = "Magnetic Tundra";
                biomeEffect = "Random Bcast Dir";
                break;
        }
        raylib::Text(biomeName, 16, raylib::Color(60, 40, 30, 255),
                     AssetManager::getInstance().getFont("TextFont"), 1.5f)
            .Draw(_bounds.x + 15, (float)yOffset);
        yOffset += 20;
        if (!biomeEffect.empty()) {
            raylib::Text(biomeEffect, 12, raylib::Color(150, 60, 30, 255),
                         AssetManager::getInstance().getFont("TextFont"), 1.5f)
                .Draw(_bounds.x + 15, (float)yOffset);
            yOffset += 15;
        }
    }

    yOffset += 5;

    if (inv) {
        auto drawResource = [&](const std::string& name, int count, raylib::Color col) {
            if (count > 0) {
                std::string text = name + ": " + std::to_string(count);
                raylib::Text(text, 14, col, AssetManager::getInstance().getFont("TextFont"), 1.5f)
                    .Draw(_bounds.x + 20, (float)yOffset);
                yOffset += 18;
            }
        };

        drawResource("Food", inv->food, raylib::Color(245, 160, 100, 255));         // Peach Orange
        drawResource("Linemate", inv->linemate, raylib::Color(110, 210, 120, 255)); // Mint Green
        drawResource("Deraumere", inv->deraumere,
                     raylib::Color(100, 180, 240, 255));                      // Soft Sky Blue
        drawResource("Sibur", inv->sibur, raylib::Color(190, 130, 230, 255)); // Lavender Purple
        drawResource("Mendiane", inv->mendiane, raylib::Color(240, 220, 110, 255)); // Warm Yellow
        drawResource("Phiras", inv->phiras, raylib::Color(235, 120, 120, 255));     // Coral Red
        drawResource("Thystame", inv->thystame, raylib::Color(245, 245, 245, 255)); // Bright White
    }

    yOffset += 10;
    raylib::Text("Entities:", 18, raylib::Color(40, 70, 100, 255),
                 AssetManager::getInstance().getFont("TextFont"), 1.5f)
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
                auto serverId = _world.get_component<ServerId>(entity);
                std::string idStr =
                    serverId ? std::to_string(serverId->id) : std::to_string(entity.id());
                std::string pInfo = "Player " + idStr;
                if (level) {
                    pInfo += " (Lvl " + std::to_string(level->level) + ")";
                }
                raylib::Text(pInfo, 16, raylib::Color(150, 40, 40, 255),
                             AssetManager::getInstance().getFont("TextFont"), 1.5f)
                    .Draw(_bounds.x + 20, (float)yOffset);
                yOffset += 18;
                if (team) {
                    raylib::Text(team->_team_name, 14, raylib::Color(100, 80, 80, 255),
                                 AssetManager::getInstance().getFont("TextFont"), 1.5f)
                        .Draw(_bounds.x + 35, (float)yOffset);
                    yOffset += 16;
                }
            }
        }
    }

    if (!foundPlayer) {
        raylib::Text("None", 16, raylib::Color(100, 80, 80, 255),
                     AssetManager::getInstance().getFont("TextFont"), 1.5f)
            .Draw(_bounds.x + 20, (float)yOffset);
    }
}

} // namespace zappy
