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

#include "Systems/RenderSystem.hpp"
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

RenderSystem::RenderSystem() {
    _camera.position = (raylib::Vector3){20.0f, 20.0f, 20.0f};
    _camera.target = (raylib::Vector3){0.0f, 0.0f, 0.0f};
    _camera.up = (raylib::Vector3){0.0f, 1.0f, 0.0f};
    _camera.fovy = 45.0f;
    _camera.projection = CAMERA_PERSPECTIVE;
}

void RenderSystem::centerCamera(int width, int height) {
    _camera.target = (raylib::Vector3){(float)width / 2.0f, 0.0f, (float)height / 2.0f};
    _camera.position =
        (raylib::Vector3){(float)width / 2.0f + 10.0f, 15.0f, (float)height / 2.0f + 10.0f};
}

void RenderSystem::update(World& w, float dt) {
    _handleInput(w, dt);
    _updateHoverState();

    auto incantationStorage = w.get_storage<ComponentIncantationEffect>();
    if (incantationStorage) {
        for (auto& [entity, effect] : *incantationStorage) {
            effect->timeElapsed += dt;
        }
    }

    // Update player HP based on server frequency
    float freq = 100.0f;
    auto timeStorage = w.get_storage<TimeUnit>();
    if (timeStorage && timeStorage->begin() != timeStorage->end()) {
        freq = (float)timeStorage->begin()->second->frequency;
    }

    auto invStorage = w.get_storage<Inventory>();
    if (invStorage) {
        for (auto& [entity, inv] : *invStorage) {
            if (inv->exactHp > 0.0f) {
                inv->exactHp -= freq * dt;
                if (inv->exactHp < 0.0f) {
                    inv->exactHp = 0.0f;
                }
            }
        }
    }
}

void RenderSystem::render(World& w) {
    _lazyLoadAssets();
    render::g_instanceBatches.clear();

    _renderBackground(w);
    _camera.BeginMode();
    _renderTerrain(w);
    _renderLandmarks(w);
    _renderResources(w);
    _renderAnimatedResources(w);
    _renderInhabitants(w);
    _renderEggs(w);
    _renderPOV(w);
    _renderIncantations(w);
    _renderParticles(w);
    _renderTombs(w);
    _renderDebugHud(w);

    // Hardware Instancing Rendering Phase
    // Iterate through batches of grouped models and pass their accumulated
    // transformation matrices to the GPU via DrawMeshInstanced, significantly
    for (auto& [key, transforms] : render::g_instanceBatches) {
        if (transforms.empty()) {
            continue;
        }
        raylib::Model& model = AssetManager::getInstance().getModel(key.modelName);
        Color tint = GetColor(key.tint);

        for (int i = 0; i < model.meshCount; i++) {
            Color oldColor =
                model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;
            Color combinedColor = {(unsigned char)((oldColor.r * tint.r) / 255),
                                   (unsigned char)((oldColor.g * tint.g) / 255),
                                   (unsigned char)((oldColor.b * tint.b) / 255),
                                   (unsigned char)((oldColor.a * tint.a) / 255)};

            // Skip tinting the head mesh (index 3) of the mannequin so it stays white
            if (key.modelName == "robot" && i == 3) {
                combinedColor = oldColor;
            }

            model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = combinedColor;

            DrawMeshInstanced(model.meshes[i], model.materials[model.meshMaterial[i]],
                              transforms.data(), transforms.size());

            model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = oldColor;
        }
    }

    _camera.EndMode();

    if (_showDebugHud) {
        _renderDebugHud(w);
    }
}

void RenderSystem::_lazyLoadAssets() {
    static bool loaded = false;
    if (!loaded) {
        AssetManager::getInstance().loadAll();
        raylib::Window::HideCursor();
        loaded = true;
    }
}

void RenderSystem::_handleInput(World& w, float dt) {
    float moveSpeed = 20.0f * dt;
    float rotateSpeed = 2.0f * dt;

    raylib::Vector3 forward = (raylib::Vector3)_camera.target - (raylib::Vector3)_camera.position;
    raylib::Vector3 right = forward.CrossProduct(_camera.up).Normalize();
    raylib::Vector3 up = right.CrossProduct(forward).Normalize();

    // Yaw Rotation (Q/E)
    if (raylib::Keyboard::IsKeyDown(KEY_Q)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        relPos = ::Vector3RotateByAxisAngle(relPos, {0, 1, 0}, rotateSpeed);
        _camera.position = (raylib::Vector3)_camera.target + relPos;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_E)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        relPos = ::Vector3RotateByAxisAngle(relPos, {0, 1, 0}, -rotateSpeed);
        _camera.position = (raylib::Vector3)_camera.target + relPos;
    }

    // Pitch Rotation (R/F)
    if (raylib::Keyboard::IsKeyDown(KEY_R)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        raylib::Vector3 nextRelPos = ::Vector3RotateByAxisAngle(relPos, right, -rotateSpeed);
        if (::Vector3Angle(nextRelPos, {0, 1, 0}) > 0.1f) {
            _camera.position = (raylib::Vector3)_camera.target + nextRelPos;
        }
    }
    if (raylib::Keyboard::IsKeyDown(KEY_F)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        raylib::Vector3 nextRelPos = ::Vector3RotateByAxisAngle(relPos, right, rotateSpeed);
        if (::Vector3Angle(nextRelPos, {0, 1, 0}) < 1.5f) {
            _camera.position = (raylib::Vector3)_camera.target + nextRelPos;
        }
    }

    // Panning (WASD)
    forward = (raylib::Vector3)_camera.target - (raylib::Vector3)_camera.position;
    forward.y = 0;
    forward = forward.Normalize();
    right = forward.CrossProduct(_camera.up);
    up = right.CrossProduct(forward).Normalize();

    if (raylib::Keyboard::IsKeyDown(KEY_W)) {
        _camera.position = (raylib::Vector3)_camera.position + forward * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target + forward * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_S)) {
        _camera.position = (raylib::Vector3)_camera.position - forward * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target - forward * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_A)) {
        _camera.position = (raylib::Vector3)_camera.position - right * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target - right * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_D)) {
        _camera.position = (raylib::Vector3)_camera.position + right * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target + right * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_LEFT_CONTROL)) {
        _camera.position = (raylib::Vector3)_camera.position - up * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target - up * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_SPACE)) {
        _camera.position = (raylib::Vector3)_camera.position + up * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target + up * moveSpeed;
    }

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT)) {
        _selectedX = _hoveredX;
        _selectedZ = _hoveredZ;
        _selectedPlayer = std::nullopt;

        // Player Selection Raycast
        raylib::Ray ray = _camera.GetMouseRay(raylib::Mouse::GetPosition());
        auto moveStorage = w.get_storage<MovementInterpolation2D>();
        if (moveStorage) {
            float closestDist = 999999.0f;
            for (auto const& [entity, move] : *moveStorage) {
                if (w.get_component<InhabitantTag>(entity)) {
                    ::BoundingBox bbox;
                    bbox.min = (::Vector3){move->visualX - 0.6f, 1.0f, move->visualY - 0.6f};
                    bbox.max = (::Vector3){move->visualX + 0.6f, 4.5f, move->visualY + 0.6f};
                    ::RayCollision collision = ::GetRayCollisionBox(ray, bbox);
                    if (collision.hit && collision.distance < closestDist) {
                        closestDist = collision.distance;
                        _selectedPlayer = entity;
                    }
                }
            }
        }

        // Fallback: if we didn't hit the 3D model precisely, but clicked on a tile with a player
        if (!_selectedPlayer.has_value() && _selectedX != InvalidTileCoord &&
            _selectedZ != InvalidTileCoord) {
            auto posStorage = w.get_storage<Position>();
            if (posStorage) {
                for (auto const& [entity, pos] : *posStorage) {
                    if (pos && pos->x == _selectedX && pos->y == _selectedZ &&
                        w.get_component<InhabitantTag>(entity)) {
                        _selectedPlayer = entity;
                        break;
                    }
                }
            }
        }

        if (_selectedPlayer.has_value()) {
            w.add_component<RequestPlayerLevel>(_selectedPlayer.value(), {});
            w.add_component<RequestPlayerInventory>(_selectedPlayer.value(), {});
        }
    }

    _showDebugHud =
        raylib::Keyboard::IsKeyDown(KEY_LEFT_SHIFT) || raylib::Keyboard::IsKeyDown(KEY_RIGHT_SHIFT);

    // Zoom (Mouse Wheel)
    float wheel = raylib::Mouse::GetWheelMove();
    if (wheel != 0) {
        raylib::Ray ray = _camera.GetMouseRay(raylib::Mouse::GetPosition());
        if (ray.direction.y != 0) {
            float t = (1.5f - ray.position.y) / ray.direction.y;
            if (t > 0) {
                raylib::Vector3 mousePoint =
                    (raylib::Vector3)ray.position + (raylib::Vector3)ray.direction * t;
                raylib::Vector3 zoomVec =
                    (mousePoint - (raylib::Vector3)_camera.position) * (wheel * 0.1f);
                float nextDist = ((raylib::Vector3)_camera.position + zoomVec).Distance(mousePoint);
                if (nextDist > 2.0f && nextDist < 100.0f) {
                    _camera.position = (raylib::Vector3)_camera.position + zoomVec;
                    _camera.target = (raylib::Vector3)_camera.target + zoomVec;
                }
            }
        }
    }
}

void RenderSystem::_updateHoverState() {
    raylib::Ray mouseRay = _camera.GetMouseRay(raylib::Mouse::GetPosition());
    _hoveredX = InvalidTileCoord;
    _hoveredZ = InvalidTileCoord;
    if (mouseRay.direction.y != 0) {
        float t = (2.0f - mouseRay.position.y) / mouseRay.direction.y;
        if (t > 0) {
            raylib::Vector3 p =
                (raylib::Vector3)mouseRay.position + (raylib::Vector3)mouseRay.direction * t;
            _hoveredX = (int)std::round(p.x);
            _hoveredZ = (int)std::round(p.z);
        }
    }
}

} // namespace zappy
