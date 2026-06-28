/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIHudPanel.cpp
*/

#include "UI/UIHudPanel.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Components/ComponentTile.hpp"
#include "Components/FollowingEntity.hpp"
#include "Graphics/AssetManager.hpp"
#include <string>

#include <limits>

namespace zappy {

UIHudPanel::UIHudPanel(raylib::Rectangle bounds, World& world, RenderSystem& renderSystem,
                       std::function<void()> onClick, int zIndex)
    : AUIComponent(bounds, onClick, zIndex), _world(world), _renderSystem(renderSystem) {}

void UIHudPanel::update(float dt, raylib::Vector2 mousePos,
                        std::shared_ptr<std::vector<UIEvent>> events) {
    AUIComponent::update(dt, mousePos, events);
    auto [selX, selZ] = _renderSystem.getSelectedTile();
    if (selX != std::numeric_limits<int>::min() && _isHovered &&
        raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT)) {
        _renderSystem.consumeClick();
    }
}

void UIHudPanel::render() {
    auto [selX, selZ] = _renderSystem.getSelectedTile();

    if (selX == std::numeric_limits<int>::min()) {
        return;
    }

    auto& tex = AssetManager::getInstance().getTexture("hud_bg");
    if (tex.id != 0) {
        ::NPatchInfo npatchInfo = {
            (raylib::Rectangle){0.0f, 0.0f, (float)tex.width, (float)tex.height},
            30,
            30,
            30,
            30,
            NPATCH_NINE_PATCH};
        tex.Draw(npatchInfo, _bounds, raylib::Vector2(0, 0), 0.0f, raylib::Color::White());
    } else {
        _bounds.Draw(raylib::Color(15, 20, 40, 230));
        _bounds.DrawLines(raylib::Color(0, 100, 255, 255), 2.0f);
    }

    auto drawTextShadow = [](const std::string& txt, float x, float y, float size,
                             raylib::Color col) {
        raylib::Text(txt, size, raylib::Color::Black(),
                     AssetManager::getInstance().getFont("TextFont"), 1.5f)
            .Draw(x + 1, y + 1);
        raylib::Text(txt, size, col, AssetManager::getInstance().getFont("TextFont"), 1.5f)
            .Draw(x, y);
    };

    drawTextShadow("Tile [" + std::to_string(selX) + ", " + std::to_string(selZ) + "]",
                   _bounds.x + 15, _bounds.y + 15, 18, raylib::Color(255, 200, 100, 255));

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

    int yOffset = _bounds.y + 40;

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
            case TerrainType::WORMHOLE:
                biomeName = "Wormhole";
                biomeEffect = "Teleports player";
                break;
        }
        drawTextShadow(biomeName, _bounds.x + 15, (float)yOffset, 16, raylib::Color::White());
        yOffset += 20;
        if (!biomeEffect.empty()) {
            drawTextShadow(biomeEffect, _bounds.x + 15, (float)yOffset, 12,
                           raylib::Color(200, 150, 100, 255));
            yOffset += 15;
        }
    }

    yOffset += 5;

    if (inv) {
        auto drawResource = [&](const std::string& name, int count, raylib::Color col) {
            if (count > 0) {
                std::string text = name + ": " + std::to_string(count);
                drawTextShadow(text, _bounds.x + 20, (float)yOffset, 14, col);
                yOffset += 18;
            }
        };

        drawResource("Food", inv->food, raylib::Color(255, 180, 120, 255));         // Peach Orange
        drawResource("Linemate", inv->linemate, raylib::Color(140, 240, 150, 255)); // Mint Green
        drawResource("Deraumere", inv->deraumere,
                     raylib::Color(130, 200, 255, 255));                      // Soft Sky Blue
        drawResource("Sibur", inv->sibur, raylib::Color(210, 150, 255, 255)); // Lavender Purple
        drawResource("Mendiane", inv->mendiane, raylib::Color(255, 240, 140, 255)); // Warm Yellow
        drawResource("Phiras", inv->phiras, raylib::Color(255, 140, 140, 255));     // Coral Red
        drawResource("Thystame", inv->thystame, raylib::Color(255, 255, 255, 255)); // Bright White
    }

    int eggCount = 0;
    auto posStorage = _world.get_storage<Position>();
    if (posStorage) {
        for (auto const& [entity, pos] : *posStorage) {
            if (pos && pos->x == selX && pos->y == selZ && _world.get_component<EggTag>(entity)) {
                eggCount++;
            }
        }
    }

    if (eggCount > 0) {
        drawTextShadow("Eggs: " + std::to_string(eggCount), _bounds.x + 20, (float)yOffset, 14,
                       raylib::Color(255, 230, 180, 255));
        yOffset += 18;
    }

    yOffset += 10;
    drawTextShadow("Entities:", _bounds.x + 15, (float)yOffset, 16, raylib::Color::White());
    yOffset += 25;

    bool foundPlayer = false;
    raylib::Vector2 mousePos = raylib::Mouse::GetPosition();
    auto followStorage = _world.get_storage<FollowingEntity>();
    bool isFollowingAny = followStorage && followStorage->size() > 0;
    Entity followedEntity = isFollowingAny ? followStorage->begin()->first : Entity(0, 0);

    if (posStorage) {
        for (auto const& [entity, pos] : *posStorage) {
            if (pos && pos->x == selX && pos->y == selZ &&
                _world.get_component<InhabitantTag>(entity)) {
                foundPlayer = true;
                auto serverId = _world.get_component<ServerId>(entity);
                std::string idStr =
                    serverId ? std::to_string(serverId->id) : std::to_string(entity.id());

                raylib::Rectangle row = {_bounds.x + 15, (float)yOffset, _bounds.width - 30, 16};
                bool isHovered = mousePos.CheckCollision(row);
                bool isFollowed = isFollowingAny && followedEntity == entity;

                drawTextShadow("Player " + idStr, _bounds.x + 30, (float)yOffset, 14,
                               raylib::Color(255, 120, 120, 255));

                if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT) && isHovered) {
                    if (followStorage) {
                        followStorage->clear();
                    }
                    if (!isFollowed) {
                        _world.add_component<FollowingEntity>(entity,
                                                              FollowingEntity{.entity = entity});
                    }
                }

                yOffset += 16;
            }
        }
    }

    if (!foundPlayer) {
        drawTextShadow("No Players", _bounds.x + 30, (float)yOffset, 14,
                       raylib::Color(200, 200, 200, 150));
    }

    auto selectedPlayer = _renderSystem.getSelectedPlayer();
    if (selectedPlayer.has_value()) {
        Entity ePlayer = selectedPlayer.value();
        auto level = _world.get_component<Level>(ePlayer);
        auto team = _world.get_component<TeamName>(ePlayer);
        auto pinv = _world.get_component<Inventory>(ePlayer);
        if (level || team || pinv) {
            float screenW = ::GetScreenWidth();
            float screenH = ::GetScreenHeight();
            float hudW = 600;
            float hudH = 120;
            raylib::Rectangle btmHud = {screenW / 2.0f - hudW / 2.0f, screenH - hudH - 20, hudW,
                                        hudH};

            auto& bgTex = AssetManager::getInstance().getTexture("hud_bg");
            if (bgTex.id != 0) {
                ::NPatchInfo npatchInfo = {
                    (raylib::Rectangle){0.0f, 0.0f, (float)bgTex.width, (float)bgTex.height},
                    30,
                    30,
                    30,
                    30,
                    NPATCH_NINE_PATCH};
                bgTex.Draw(npatchInfo, btmHud, raylib::Vector2(0, 0), 0.0f, raylib::Color::White());
            } else {
                btmHud.Draw(raylib::Color(15, 20, 40, 240));
                btmHud.DrawLines(raylib::Color(200, 150, 50, 255), 3.0f);
            }

            auto serverId = _world.get_component<ServerId>(ePlayer);
            std::string idStr =
                serverId ? std::to_string(serverId->id) : std::to_string(ePlayer.id());

            // Shadow text renderer (reused from top of render())

            // Left Panel: Identity
            auto& iconTeam = AssetManager::getInstance().getTexture("hud_icon_team");
            if (iconTeam.id != 0) {
                iconTeam.Draw(
                    raylib::Rectangle{0, 0, (float)iconTeam.width, (float)iconTeam.height},
                    raylib::Rectangle{btmHud.x + 20, btmHud.y + 20, 32, 32}, raylib::Vector2{0, 0},
                    0.0f, team ? team->_color : raylib::Color::White());
            }
            drawTextShadow("PLAYER " + idStr, btmHud.x + 60, btmHud.y + 25, 22,
                           raylib::Color::White());

            if (team) {
                drawTextShadow(team->_team_name, btmHud.x + 60, btmHud.y + 50, 16, team->_color);
            }

            if (level) {
                auto& iconLevel = AssetManager::getInstance().getTexture("hud_icon_level");
                if (iconLevel.id != 0) {
                    iconLevel.Draw(
                        raylib::Rectangle{0, 0, (float)iconLevel.width, (float)iconLevel.height},
                        raylib::Rectangle{btmHud.x + 20, btmHud.y + 70, 32, 32},
                        raylib::Vector2{0, 0}, 0.0f, raylib::Color::White());
                }
                drawTextShadow("Lv. " + std::to_string(level->level), btmHud.x + 60, btmHud.y + 75,
                               20, raylib::Color(255, 220, 100, 255));
            }

            // HP Bar (Center)
            if (pinv) {
                float hp = pinv->exactHp;
                float hpPercentage = 0.0f;
                if (pinv->maxHp > 0.0f) {
                    hpPercentage = hp / pinv->maxHp;
                }
                if (hpPercentage > 1.0f) {
                    hpPercentage = 1.0f;
                }

                // Moved slightly to the right so it doesn't overlap with long names
                float hpXOffset = btmHud.x + 230;

                drawTextShadow(std::to_string((int)hp) + " HP", hpXOffset, btmHud.y + 35, 20,
                               raylib::Color(255, 100, 100, 255));

                auto& barBg = AssetManager::getInstance().getTexture("hud_bar_bg");
                auto& barFill = AssetManager::getInstance().getTexture("hud_bar_fill");
                if (barBg.id != 0 && barFill.id != 0) {
                    ::NPatchInfo npBg = {
                        (raylib::Rectangle){0.0f, 0.0f, (float)barBg.width, (float)barBg.height},
                        10,
                        10,
                        10,
                        10,
                        NPATCH_NINE_PATCH};
                    raylib::Rectangle barRect = {hpXOffset, btmHud.y + 70, 150, 24};
                    barBg.Draw(npBg, barRect, raylib::Vector2(0, 0), 0.0f, raylib::Color::White());

                    if (hpPercentage > 0.0f) {
                        raylib::Rectangle fillRect = {barRect.x + 4, barRect.y + 4,
                                                      (barRect.width - 8) * hpPercentage,
                                                      barRect.height - 8};
                        raylib::Rectangle sourceRect = {0, 0, (float)barFill.width * hpPercentage,
                                                        (float)barFill.height};
                        barFill.Draw(sourceRect, fillRect, raylib::Vector2(0, 0), 0.0f,
                                     raylib::Color(255, 100, 100, 255));
                    }
                }

                // Right Panel: Inventory Hotbar
                float rx = btmHud.x + 400;
                float ry = btmHud.y + 20;
                auto drawResSlot = [&](const std::string& name, int count, raylib::Color col,
                                       float x, float y) {
                    ::DrawRectangleRounded(raylib::Rectangle{x, y, 20, 20}, 0.5f, 16, col);
                    ::DrawRectangleRoundedLinesEx(raylib::Rectangle{x, y, 20, 20}, 0.5f, 16, 2.0f,
                                                  raylib::Color::White());
                    drawTextShadow(std::to_string(count), x + 26, y + 1, 18,
                                   raylib::Color::White());
                };
                drawResSlot("L", pinv->linemate, raylib::Color(110, 210, 120, 255), rx, ry);
                drawResSlot("D", pinv->deraumere, raylib::Color(100, 180, 240, 255), rx + 70, ry);
                drawResSlot("S", pinv->sibur, raylib::Color(190, 130, 230, 255), rx + 140, ry);
                drawResSlot("M", pinv->mendiane, raylib::Color(240, 220, 110, 255), rx, ry + 35);
                drawResSlot("P", pinv->phiras, raylib::Color(235, 120, 120, 255), rx + 70, ry + 35);
                drawResSlot("T", pinv->thystame, raylib::Color(245, 245, 245, 255), rx + 140,
                            ry + 35);
            }
        }
    }
}

} // namespace zappy
