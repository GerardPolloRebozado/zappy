/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** RenderSystem.cpp
*/

/**
 * @file RenderSystem.cpp
 * @brief Implementation of the RenderSystem class.
 */

#include "Color.hpp"
#include "Components/ComponentIncantationEffect.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Components/ComponentTile.hpp"
#include "Components/FollowingEntity.hpp"
#include "Core.hpp"
#include "ECS/World.hpp"
#include "Graphics/TileTextures.hpp"
#include "Graphics/VoxelBatcher.hpp"
#include "Keyboard.hpp"
#include "Systems/RenderSystem.hpp"
#include "Vector3.hpp"
#include "raylib.h"

#include "Systems/RenderSystemBatches.hpp"
#include "UI/UIText.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <ranges>
#include <raymath.h>
#include <rlgl.h>
#include <set>
#include <unordered_set>

namespace zappy {

void RenderSystem::_renderParticles(World& w) {
    auto emitterStorage = w.get_storage<ComponentParticleEmitter>();
    if (!emitterStorage) {
        return;
    }

    raylib::Vector3 forward =
        ((raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target).Normalize();
    raylib::Vector3 right = ((raylib::Vector3)_camera.up).CrossProduct(forward).Normalize();
    raylib::Vector3 up = forward.CrossProduct(right).Normalize();

    ::rlDisableDepthMask();

    for (auto const& [entity, emitterPtr] : *emitterStorage) {
        if (emitterPtr->particles.empty()) {
            continue;
        }

        bool hasTexture = !emitterPtr->spriteName.empty();
        unsigned int texID = 0;
        if (hasTexture) {
            auto& tex = AssetManager::getInstance().getTexture(emitterPtr->spriteName);
            texID = tex.id;
        }

        if (hasTexture) {
            ::rlSetTexture(texID);
        }

        ::rlBegin(RL_QUADS);

        for (const auto& p : emitterPtr->particles) {
            float lifePct = 1.0f - (p.lifeRemaining / p.lifetime);
            if (lifePct < 0.0f) {
                lifePct = 0.0f;
            }
            if (lifePct > 1.0f) {
                lifePct = 1.0f;
            }

            unsigned char r =
                (unsigned char)(p.startColor.r + (p.endColor.r - p.startColor.r) * lifePct);
            unsigned char g =
                (unsigned char)(p.startColor.g + (p.endColor.g - p.startColor.g) * lifePct);
            unsigned char b =
                (unsigned char)(p.startColor.b + (p.endColor.b - p.startColor.b) * lifePct);
            unsigned char a =
                (unsigned char)(p.startColor.a + (p.endColor.a - p.startColor.a) * lifePct);

            float s = (p.startSize + (p.endSize - p.startSize) * lifePct) / 2.0f;
            float x = p.position.x;
            float y = p.position.y;
            float z = p.position.z;

            ::rlColor4ub(r, g, b, a);

            if (hasTexture) {
                raylib::Vector3 p1 = p.position - right * s - up * s;
                raylib::Vector3 p2 = p.position + right * s - up * s;
                raylib::Vector3 p3 = p.position + right * s + up * s;
                raylib::Vector3 p4 = p.position - right * s + up * s;

                ::rlTexCoord2f(0.0f, 1.0f);
                ::rlVertex3f(p1.x, p1.y, p1.z);
                ::rlTexCoord2f(1.0f, 1.0f);
                ::rlVertex3f(p2.x, p2.y, p2.z);
                ::rlTexCoord2f(1.0f, 0.0f);
                ::rlVertex3f(p3.x, p3.y, p3.z);
                ::rlTexCoord2f(0.0f, 0.0f);
                ::rlVertex3f(p4.x, p4.y, p4.z);
            } else {
                // Front
                ::rlVertex3f(x - s, y - s, z + s);
                ::rlVertex3f(x + s, y - s, z + s);
                ::rlVertex3f(x + s, y + s, z + s);
                ::rlVertex3f(x - s, y + s, z + s);

                // Back
                ::rlVertex3f(x - s, y - s, z - s);
                ::rlVertex3f(x - s, y + s, z - s);
                ::rlVertex3f(x + s, y + s, z - s);
                ::rlVertex3f(x + s, y - s, z - s);

                // Top
                ::rlVertex3f(x - s, y + s, z - s);
                ::rlVertex3f(x - s, y + s, z + s);
                ::rlVertex3f(x + s, y + s, z + s);
                ::rlVertex3f(x + s, y + s, z - s);

                // Bottom
                ::rlVertex3f(x - s, y - s, z - s);
                ::rlVertex3f(x + s, y - s, z - s);
                ::rlVertex3f(x + s, y - s, z + s);
                ::rlVertex3f(x - s, y - s, z + s);

                // Right
                ::rlVertex3f(x + s, y - s, z - s);
                ::rlVertex3f(x + s, y + s, z - s);
                ::rlVertex3f(x + s, y + s, z + s);
                ::rlVertex3f(x + s, y - s, z + s);

                // Left
                ::rlVertex3f(x - s, y - s, z - s);
                ::rlVertex3f(x - s, y - s, z + s);
                ::rlVertex3f(x - s, y + s, z + s);
                ::rlVertex3f(x - s, y + s, z - s);
            }
        }
        ::rlEnd();

        if (hasTexture) {
            ::rlSetTexture(0);
        }
    }

    ::rlEnableDepthMask();
}

void RenderSystem::_renderIncantations(World& w) {
    auto incantationStorage = w.get_storage<ComponentIncantationEffect>();
    if (!incantationStorage) {
        return;
    }

    ::rlDisableDepthMask();

    for (const auto& [entity, effect] : *incantationStorage) {
        float x = (float)effect->x;
        float z = (float)effect->y;
        float time = effect->timeElapsed;

        // Fade in over 1.5 seconds
        float fadeIn = std::min(time / 1.5f, 1.0f);

        raylib::Shader& shader = AssetManager::getInstance().getShader("incantation_aura");
        int timeLoc = ::GetShaderLocation(shader, "time");
        ::SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

        ::BeginShaderMode(shader);

        unsigned char alpha = (unsigned char)(255.0f * fadeIn);
        raylib::Color color = raylib::Color(255, 255, 255, alpha);

        ::rlDisableBackfaceCulling();
        ::rlDisableDepthMask();

        // Draw a flat quad on the ground for the magic circle
        float yBase = 2.02f; // Slightly above terrain to avoid Z-fighting
        float size = 0.6f;   // Total diameter 1.2 (radius 0.6)

        ::rlBegin(RL_QUADS);
        ::rlColor4ub(color.r, color.g, color.b, color.a);

        ::rlTexCoord2f(0.0f, 0.0f);
        ::rlVertex3f(x - size, yBase, z - size);
        ::rlTexCoord2f(0.0f, 1.0f);
        ::rlVertex3f(x - size, yBase, z + size);
        ::rlTexCoord2f(1.0f, 1.0f);
        ::rlVertex3f(x + size, yBase, z + size);
        ::rlTexCoord2f(1.0f, 0.0f);
        ::rlVertex3f(x + size, yBase, z - size);

        ::rlEnd();

        ::rlEnableDepthMask();
        ::rlEnableBackfaceCulling();

        ::EndShaderMode();

        ::rlDisableDepthMask();
        for (Entity player : effect->participants) {
            auto move = w.get_component<MovementInterpolation2D>(player);
            if (move) {
                float pulse = (std::sin(time * 5.0f) + 1.0f) * 0.5f;
                unsigned char pAlpha = (unsigned char)((30.0f + 20.0f * pulse) * fadeIn);
                raylib::Color innerColor = raylib::Color(255, 255, 255, pAlpha);
                raylib::Color outerColor = raylib::Color(200, 230, 255, pAlpha / 2);

                // Draw layered spheres to create a soft, glowing aura around the body
                // The center of the mannequin is roughly at y = 2.6
                ::DrawSphere(raylib::Vector3{move->visualX, 2.6f, move->visualY}, 0.45f,
                             innerColor);
                ::DrawSphere(raylib::Vector3{move->visualX, 2.6f, move->visualY}, 0.55f,
                             outerColor);
            }
        }
    }

    ::rlEnableDepthMask();
}

void RenderSystem::_renderHoverEffect(int x, int z) {
    raylib::Color hCol = raylib::Color::White();
    float wH = 0.15f;
    float wT = 0.05f;
    float yB = 2.0f;

    raylib::Vector3((float)x, yB + wH / 2, (float)z + 0.5f).DrawCube(1.0f + wT, wH, wT, hCol);
    raylib::Vector3((float)x, yB + wH / 2, (float)z - 0.5f).DrawCube(1.0f + wT, wH, wT, hCol);
    raylib::Vector3((float)x + 0.5f, yB + wH / 2, (float)z).DrawCube(wT, wH, 1.0f + wT, hCol);
    raylib::Vector3((float)x - 0.5f, yB + wH / 2, (float)z).DrawCube(wT, wH, 1.0f + wT, hCol);
}

void RenderSystem::_renderPOV(World& w) {
    auto followingEntity = w.get_storage<FollowingEntity>();
    if (!followingEntity || followingEntity->size() == 0) {
        return;
    }

    if (raylib::Keyboard::IsKeyDown(KEY_W) || raylib::Keyboard::IsKeyDown(KEY_A) ||
        raylib::Keyboard::IsKeyDown(KEY_S) || raylib::Keyboard::IsKeyDown(KEY_D)) {
        followingEntity->clear();
        return;
    }
    auto entity = followingEntity->begin()->first;
    auto pos = w.get_component<Position>(entity);
    auto orientation = w.get_component<Orientation>(entity);
    if (!pos || !orientation) {
        return;
    }

    Vector3 headPos = {static_cast<float>(pos->x), 3.5f, static_cast<float>(pos->y)};
    Vector3 lookDir = {0.0f, 0.0f, 0.0f};

    switch (orientation->current_direction) {
        case Orientation::N:
            lookDir = {0.0f, 0.0f, -1.0f};
            break;
        case Orientation::E:
            lookDir = {1.0f, 0.0f, 0.0f};
            break;
        case Orientation::S:
            lookDir = {0.0f, 0.0f, 1.0f};
            break;
        case Orientation::W:
            lookDir = {-1.0f, 0.0f, 0.0f};
            break;
    }

    _camera.position = headPos;
    _camera.target = Vector3Add(headPos, lookDir);
}

void RenderSystem::_renderDebugHud(World& w) {
    int padding = 15;
    int y = padding;
    int x = padding;
    int spacing = 25;

    auto drawText = [&](const std::string& text) {
        ::DrawText(text.c_str(), x, y, 20, WHITE);
        y += spacing;
    };

    ::DrawRectangle(5, 5, 450, 300, raylib::Color(0, 0, 0, 150));

    drawText("--- DEBUG HUD (HOLD SHIFT) ---");
    drawText("FPS: " + std::to_string(::GetFPS()));

    char camPos[64];
    snprintf(camPos, sizeof(camPos), "Camera Pos: %.2f, %.2f, %.2f", _camera.position.x,
             _camera.position.y, _camera.position.z);
    drawText(camPos);

    char camTgt[64];
    snprintf(camTgt, sizeof(camTgt), "Camera Tgt: %.2f, %.2f, %.2f", _camera.target.x,
             _camera.target.y, _camera.target.z);
    drawText(camTgt);

    drawText("Hovered Tile: " + std::to_string(_hoveredX) + ", " + std::to_string(_hoveredZ));
    drawText("Selected Tile: " + std::to_string(_selectedX) + ", " + std::to_string(_selectedZ));

    auto terrainStorage = w.get_storage<TerrainType>();
    drawText("Tiles Loaded: " + std::to_string(terrainStorage ? terrainStorage->size() : 0));

    auto inhabitantStorage = w.get_storage<Orientation>();
    drawText("Inhabitants: " + std::to_string(inhabitantStorage ? inhabitantStorage->size() : 0));

    auto eggStorage = w.get_storage<Egg>();
    drawText("Eggs on Map: " + std::to_string(eggStorage ? eggStorage->size() : 0));
}

void RenderSystem::_reanderMapEvents(World& w, std::string event) {
    if (event == "meteor_shower") {
        std::cout << "-------------------------------------------------------" << std::endl;
        int animFrames = 0;
        Image imScarfyAnim = LoadImageAnim("assets/textures/Meteorite/FB000.gif", &animFrames);
        Texture2D texScarfyAnim = LoadTextureFromImage(imScarfyAnim);
        DrawTexture(texScarfyAnim, GetScreenWidth() / 2 - texScarfyAnim.width / 2, 140, WHITE);
    }
}

void RenderSystem::renderShowEvents(World& w) {
    auto eventStorage = w.get_storage<MapEvent>();
    if (!eventStorage) {
        return;
    }
    float currentY = 100.0f;
    raylib::Vector2 pos = {1920.0f / 2.0f - 200.0f, currentY};

    for (const auto& [entity, mapEvent] : *eventStorage) {
        if (mapEvent->active && mapEvent->name != "none") {
            auto& font = AssetManager::getInstance().getFont("HeaderFont");

            std::string text = mapEvent->name;
            std::transform(text.begin(), text.end(), text.begin(), ::toupper);
            raylib::Rectangle::Draw(pos.x - 10, pos.y - 10, text.length() * 37, 80,
                                    raylib::Color{255, 255, 255, 130});
            font.DrawText(text, pos, 60, 1.5f, raylib::Color::DarkPurple());
            currentY += 70.0f;

            RenderSystem::_reanderMapEvents(w, mapEvent->name);
        }
    }
}

void RenderSystem::_renderWormholes(World& w) {
    auto terrainStorage = w.get_storage<TerrainType>();
    if (!terrainStorage) {
        return;
    }

    raylib::Shader& shader = AssetManager::getInstance().getShader("wormhole_portal");
    int timeLoc = ::GetShaderLocation(shader, "time");
    float time = (float)GetTime();
    ::SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

    ::rlDisableBackfaceCulling();
    ::rlDisableDepthMask();
    ::BeginShaderMode(shader);

    ::rlBegin(RL_QUADS);
    ::rlColor4ub(255, 255, 255, 255);

    for (const auto& [entity, type] : *terrainStorage) {
        if (type->current_type != TerrainType::WORMHOLE) {
            continue;
        }

        auto pos = w.get_component<Position>(entity);
        if (!pos) {
            continue;
        }

        float x = (float)pos->x;
        float z = (float)pos->y;
        float yBase = 2.01f; // Slightly above tile
        float size = 0.5f;   // 1.0x1.0 square

        ::rlTexCoord2f(0.0f, 0.0f);
        ::rlVertex3f(x - size, yBase, z - size);
        ::rlTexCoord2f(0.0f, 1.0f);
        ::rlVertex3f(x - size, yBase, z + size);
        ::rlTexCoord2f(1.0f, 1.0f);
        ::rlVertex3f(x + size, yBase, z + size);
        ::rlTexCoord2f(1.0f, 0.0f);
        ::rlVertex3f(x + size, yBase, z - size);
    }

    ::rlEnd();
    ::EndShaderMode();
    ::rlEnableDepthMask();
    ::rlEnableBackfaceCulling();
}

} // namespace zappy
