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

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <ranges>
#include <raymath.h>
#include <rlgl.h>
#include <set>
#include <unordered_set>

#include "Systems/RenderSystemBatches.hpp"

namespace zappy {

void RenderSystem::_renderInhabitants(World& w) {
    auto orientationStorage = w.get_storage<Orientation>();
    if (!orientationStorage) {
        return;
    }

    auto& am = AssetManager::getInstance();
    raylib::Model& robot = const_cast<raylib::Model&>(am.getModel("robot"));
    constexpr float scale = 0.8f;

    // Get server frequency
    float freq = 100.0f;
    auto timeStorage = w.get_storage<TimeUnit>();
    if (timeStorage && timeStorage->begin() != timeStorage->end()) {
        freq = static_cast<float>(timeStorage->begin()->second->frequency);
    }
    if (freq <= 0.0f) {
        freq = 1.0f;
    }

    // Get bounding box to center the model
    BoundingBox box = robot.GetBoundingBox();
    raylib::Vector3 center = {(box.min.x + box.max.x) / 2.0f,
                              box.min.y, // Ground it on its bottom
                              (box.min.z + box.max.z) / 2.0f};

    std::set<std::string> teamNames;
    auto teamStorage = w.get_storage<TeamName>();
    if (teamStorage) {
        for (auto const& [e, t] : *teamStorage) {
            teamNames.insert(t->_team_name);
        }
    }

    for (auto const& [entity, orientationPtr] : *orientationStorage) {
        auto pos = w.get_component<Position>(entity);
        auto move = w.get_component<MovementInterpolation2D>(entity);
        auto anim = w.get_component<Animation>(entity);
        if (pos && move) {
            if (anim && !anim->currentAnim.empty()) {
                try {
                    auto& modelAnim = am.getAnimation(anim->currentAnim);
                    UpdateModelAnimation(robot, modelAnim, static_cast<int>(anim->currentFrame));
                } catch (...) {
                }
            }
            float rotation = 0.0f;
            switch (orientationPtr->current_direction) {
                case Orientation::N:
                    rotation = 270.0f - 90.0f;
                    break;
                case Orientation::E:
                    rotation = 180.0f - 90.0f;
                    break;
                case Orientation::S:
                    rotation = 90.0f - 90.0f;
                    break;
                case Orientation::W:
                    rotation = 0.0f - 90.0f;
                    break;
            }

            raylib::Vector3 centerOffset = {center.x * scale, center.y * scale, center.z * scale};
            centerOffset = ::Vector3RotateByAxisAngle(centerOffset, {0, 1, 0}, rotation * DEG2RAD);

            raylib::Vector3 vpos(move->visualX - centerOffset.x, 2.01f - centerOffset.y,
                                 move->visualY - centerOffset.z);

            auto team = w.get_component<TeamName>(entity);
            if (team) {
                raylib::Color tColor = team->_color;

                // Draw mannequin manually to keep head white
                Matrix matScale = MatrixScale(scale, scale, scale);
                Matrix matRotation = MatrixRotate({0, 1, 0}, rotation * DEG2RAD);
                Matrix matTranslation = MatrixTranslate(vpos.x, vpos.y, vpos.z);
                Matrix matTransform =
                    MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

                for (int i = 0; i < robot.meshCount; i++) {
                    Material mat = robot.materials[robot.meshMaterial[i]];
                    Color drawColor = tColor;
                    if (i == 3) {
                        drawColor = WHITE; // Head is white
                    } else {
                        drawColor = tColor;
                    }
                    mat.maps[MATERIAL_MAP_DIFFUSE].color = drawColor;
                    ::DrawMesh(robot.meshes[i], mat, matTransform);
                }

                float wH = 0.12f;
                float wT = 0.12f;
                float yB = 2.02f;
                float dist = 0.38f;
                raylib::Vector3(move->visualX, yB + wH / 2, move->visualY + dist)
                    .DrawCube(dist * 2 + wT, wH, wT, tColor);
                raylib::Vector3(move->visualX, yB + wH / 2, move->visualY - dist)
                    .DrawCube(dist * 2 + wT, wH, wT, tColor);
                raylib::Vector3(move->visualX + dist, yB + wH / 2, move->visualY)
                    .DrawCube(wT, wH, dist * 2 + wT, tColor);
                raylib::Vector3(move->visualX - dist, yB + wH / 2, move->visualY)
                    .DrawCube(wT, wH, dist * 2 + wT, tColor);
            }
        }
    }
}

void RenderSystem::_renderEggs(World& w) {
    auto eggStorage = w.get_storage<Egg>();
    if (!eggStorage) {
        return;
    }

    raylib::Model& eggModel = AssetManager::getInstance().getModel("egg");
    constexpr float scale = 0.1f;

    for (const auto& entity : *eggStorage | std::views::keys) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            const raylib::Vector3 vpos(static_cast<float>(pos->x), 2.3f,
                                       static_cast<float>(pos->y));
            render::addInstance("egg", vpos, {0, 1, 0}, 0.0f, {scale, scale, scale}, WHITE,
                                eggModel.transform);
        }
    }
}

void RenderSystem::_renderTombs(World& w) {
    auto tombStorage = w.get_storage<TombTag>();
    if (!tombStorage) {
        return;
    }

    raylib::Model& tombModel = AssetManager::getInstance().getModel("skull");
    constexpr float scale = 0.8f;

    for (const auto& entity : *tombStorage | std::views::keys) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            const raylib::Vector3 vpos(static_cast<float>(pos->x), 2.3f,
                                       static_cast<float>(pos->y));
            render::addInstance("skull", vpos, {0, 1, 0}, 0.0f, {scale, scale, scale}, WHITE,
                                tombModel.transform);
        }
    }
}

void RenderSystem::_renderCelestials(World& w) {
    auto celestialStorage = w.get_storage<CelestialObject>();
    if (!celestialStorage) {
        return;
    }

    for (const auto& entity : *celestialStorage | std::views::keys) {
        auto celestial = w.get_component<CelestialObject>(entity);
        raylib::Model& celestialModel = AssetManager::getInstance().getModel(celestial->model);
        if (celestial) {
            const raylib::Vector3 vpos(celestial->x, celestial->z,
                                       celestial->y);
            render::addInstance(celestial->model, vpos, {0, 1, 0}, 0.0f, {celestial->size, celestial->size, celestial->size}, WHITE,
                                celestialModel.transform);
        }
    }
}

void RenderSystem::_renderBackground(World& w) {
    auto backgroundStorage = w.get_storage<BackgroundParallax>();
    if (!backgroundStorage)
        return;
    for (auto& [entity, backgroundPtr] : *backgroundStorage) {
        auto& background = *backgroundPtr;

        // Unfortunately the texture is to small to cover the entire screen, so I had to add it multiple time to not deform it.
        // It may change with a bigger texture.

        // Background
        DrawTextureEx(background.background, (Vector2){background.scrollingBack, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*2 + background.scrollingBack, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*4 + background.scrollingBack, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*6 + background.scrollingBack, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*8 + background.scrollingBack, 0}, 0.0f, 2.0f, WHITE);

        DrawTextureEx(background.background, (Vector2){background.scrollingBack, background.background.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*2 + background.scrollingBack, background.background.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*4 + background.scrollingBack, background.background.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*6 + background.scrollingBack, background.background.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*8 + background.scrollingBack, background.background.height * 2.0f}, 0.0f, 2.0f, WHITE);

        DrawTextureEx(background.background, (Vector2){background.scrollingBack, background.background.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*2 + background.scrollingBack, background.background.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*4 + background.scrollingBack, background.background.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*6 + background.scrollingBack, background.background.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.background, (Vector2){background.background.width*8 + background.scrollingBack, background.background.height * 4.0f}, 0.0f, 2.0f, WHITE);

        // Midground
        DrawTextureEx(background.midground, (Vector2){background.scrollingMid, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*2 + background.scrollingMid, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*4 + background.scrollingMid, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*6 + background.scrollingMid, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*8 + background.scrollingMid, 0}, 0.0f, 2.0f, WHITE);

        DrawTextureEx(background.midground, (Vector2){background.scrollingMid, background.midground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*2 + background.scrollingMid, background.midground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*4 + background.scrollingMid, background.midground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*6 + background.scrollingMid, background.midground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*8 + background.scrollingMid, background.midground.height * 2.0f}, 0.0f, 2.0f, WHITE);

        DrawTextureEx(background.midground, (Vector2){background.scrollingMid, background.midground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*2 + background.scrollingMid, background.midground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*4 + background.scrollingMid, background.midground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*6 + background.scrollingMid, background.midground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.midground, (Vector2){background.midground.width*8 + background.scrollingMid, background.midground.height * 4.0f}, 0.0f, 2.0f, WHITE);

        // Foreground
        DrawTextureEx(background.foreground, (Vector2){background.scrollingFore, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*2 + background.scrollingFore, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*4 + background.scrollingFore, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*6 + background.scrollingFore, 0}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*8 + background.scrollingFore, 0}, 0.0f, 2.0f, WHITE);

        DrawTextureEx(background.foreground, (Vector2){background.scrollingFore, background.foreground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*2 + background.scrollingFore, background.foreground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*4 + background.scrollingFore, background.foreground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*6 + background.scrollingFore, background.foreground.height * 2.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*8 + background.scrollingFore, background.foreground.height * 2.0f}, 0.0f, 2.0f, WHITE);

        DrawTextureEx(background.foreground, (Vector2){background.scrollingFore, background.foreground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*2 + background.scrollingFore, background.foreground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*4 + background.scrollingFore, background.foreground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*6 + background.scrollingFore, background.foreground.height * 4.0f}, 0.0f, 2.0f, WHITE);
        DrawTextureEx(background.foreground, (Vector2){background.foreground.width*8 + background.scrollingFore, background.foreground.height * 4.0f}, 0.0f, 2.0f, WHITE);
    }
}

} // namespace zappy
