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
                       int zIndex)
    : AUIComponent(bounds, zIndex), _world(world), _renderSystem(renderSystem) {}

void UIHudPanel::render() {
    auto [selX, selZ] = _renderSystem.getSelectedTile();

    if (selX == std::numeric_limits<int>::min()) {
        return;
    }

    _bounds.Draw(raylib::Color(15, 20, 40, 230));
    _bounds.DrawLines(raylib::Color(0, 100, 255, 255), 2.0f);

    raylib::Text infoText("Tile [" + std::to_string(selX) + ", " + std::to_string(selZ) + "]", 20,
                          raylib::Color::RayWhite(),
                          AssetManager::getInstance().getFont("BoldPixels"), 1.5f);
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
        raylib::Text(biomeName, 16, raylib::Color::LightGray(),
                     AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
            .Draw(_bounds.x + 15, (float)yOffset);
        yOffset += 20;
        if (!biomeEffect.empty()) {
            raylib::Text(biomeEffect, 12, ORANGE, AssetManager::getInstance().getFont("BoldPixels"),
                         1.5f)
                .Draw(_bounds.x + 15, (float)yOffset);
            yOffset += 15;
        }
    }

    yOffset += 5;

    if (inv) {
        auto drawResource = [&](const std::string& name, int count, raylib::Color col) {
            if (count > 0) {
                std::string text = name + ": " + std::to_string(count);
                raylib::Text(text, 14, col, AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
                    .Draw(_bounds.x + 20, (float)yOffset);
                yOffset += 18;
            }
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
    raylib::Text("Entities:", 18, raylib::Color(0, 150, 255, 255),
                 AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
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
                raylib::Text(pInfo, 16, RED, AssetManager::getInstance().getFont("BoldPixels"),
                             1.5f)
                    .Draw(_bounds.x + 20, (float)yOffset);
                yOffset += 18;
                if (team) {
                    raylib::Text(team->team_name, 14, RAYWHITE,
                                 AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
                        .Draw(_bounds.x + 35, (float)yOffset);
                    yOffset += 16;
                }
            }
        }
    }

    if (!foundPlayer) {
        raylib::Text("None", 16, GRAY, AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
            .Draw(_bounds.x + 20, (float)yOffset);
    }
}

} // namespace zappy
