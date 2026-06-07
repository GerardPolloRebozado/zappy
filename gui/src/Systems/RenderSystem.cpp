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
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
#include "ECS/World.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

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
    (void)w;
    _handleInput(dt);
    _updateHoverState();
}

void RenderSystem::render(World& w) {
    _lazyLoadAssets();

    _camera.BeginMode();
    _renderTerrain(w);
    _renderLandmarks(w);
    _renderResources(w);
    _renderInhabitants(w);
    _camera.EndMode();
}

void RenderSystem::_lazyLoadAssets() {
    static bool loaded = false;
    if (!loaded) {
        AssetManager::getInstance().loadAll();
        HideCursor();
        loaded = true;
    }
}

void RenderSystem::_handleInput(float dt) {
    float moveSpeed = 20.0f * dt;
    float rotateSpeed = 2.0f * dt;

    raylib::Vector3 forward = (raylib::Vector3)_camera.target - (raylib::Vector3)_camera.position;
    raylib::Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, _camera.up));

    // Yaw Rotation (Q/E)
    if (IsKeyDown(KEY_Q)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        relPos = Vector3RotateByAxisAngle(relPos, {0, 1, 0}, rotateSpeed);
        _camera.position = (raylib::Vector3)_camera.target + relPos;
    }
    if (IsKeyDown(KEY_E)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        relPos = Vector3RotateByAxisAngle(relPos, {0, 1, 0}, -rotateSpeed);
        _camera.position = (raylib::Vector3)_camera.target + relPos;
    }

    // Pitch Rotation (R/F)
    if (IsKeyDown(KEY_R)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        raylib::Vector3 nextRelPos = Vector3RotateByAxisAngle(relPos, right, -rotateSpeed);
        if (Vector3Angle(nextRelPos, {0, 1, 0}) > 0.1f) {
            _camera.position = (raylib::Vector3)_camera.target + nextRelPos;
        }
    }
    if (IsKeyDown(KEY_F)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        raylib::Vector3 nextRelPos = Vector3RotateByAxisAngle(relPos, right, rotateSpeed);
        if (Vector3Angle(nextRelPos, {0, 1, 0}) < 1.5f) {
            _camera.position = (raylib::Vector3)_camera.target + nextRelPos;
        }
    }

    // Panning (WASD)
    forward = (raylib::Vector3)_camera.target - (raylib::Vector3)_camera.position;
    forward.y = 0;
    forward = Vector3Normalize(forward);
    right = Vector3CrossProduct(forward, _camera.up);

    if (IsKeyDown(KEY_W)) {
        _camera.position = (raylib::Vector3)_camera.position + forward * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target + forward * moveSpeed;
    }
    if (IsKeyDown(KEY_S)) {
        _camera.position = (raylib::Vector3)_camera.position - forward * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target - forward * moveSpeed;
    }
    if (IsKeyDown(KEY_A)) {
        _camera.position = (raylib::Vector3)_camera.position - right * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target - right * moveSpeed;
    }
    if (IsKeyDown(KEY_D)) {
        _camera.position = (raylib::Vector3)_camera.position + right * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target + right * moveSpeed;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        _selectedX = _hoveredX;
        _selectedZ = _hoveredZ;
    }

    // Zoom (Mouse Wheel)
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Ray ray = GetMouseRay(GetMousePosition(), _camera);
        if (ray.direction.y != 0) {
            float t = (1.5f - ray.position.y) / ray.direction.y;
            if (t > 0) {
                raylib::Vector3 mousePoint =
                    (raylib::Vector3)ray.position + (raylib::Vector3)ray.direction * t;
                raylib::Vector3 zoomVec =
                    (mousePoint - (raylib::Vector3)_camera.position) * (wheel * 0.1f);
                float nextDist =
                    Vector3Distance((raylib::Vector3)_camera.position + zoomVec, mousePoint);
                if (nextDist > 2.0f && nextDist < 100.0f) {
                    _camera.position = (raylib::Vector3)_camera.position + zoomVec;
                    _camera.target = (raylib::Vector3)_camera.target + zoomVec;
                }
            }
        }
    }
}

void RenderSystem::_updateHoverState() {
    Ray mouseRay = GetMouseRay(GetMousePosition(), _camera);
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

void RenderSystem::_renderTerrain(World& w) {
    auto terrainStorage = w.get_storage<TerrainType>();
    if (!terrainStorage) {
        return;
    }

    raylib::Vector3 cameraTarget = _camera.target;

    for (auto const& [entity, type] : *terrainStorage) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            // Simple distance culling: only render tiles within 60 units of camera target
            float dist = std::sqrt(std::pow(pos->x - cameraTarget.x, 2) +
                                   std::pow(pos->y - cameraTarget.z, 2));
            if (dist > 60.0f) {
                continue;
            }

            raylib::Vector3 vpos(pos->x, 1.5f, pos->y);

            raylib::Color color = GRAY;
            switch (type->current_type) {
                case TerrainType::GRASS:
                    color = raylib::Color::Green();
                    break;
                case TerrainType::MOUNTAIN:
                    color = raylib::Color::DarkGray();
                    break;
                case TerrainType::WATER:
                    color = raylib::Color::Blue();
                    break;
                case TerrainType::SAND:
                    color = raylib::Color::Gold();
                    break;
                case TerrainType::FOREST:
                    color = raylib::Color::DarkGreen();
                    break;
                case TerrainType::OBSIDIAN_BARRENS:
                    color = raylib::Color::Black();
                    break;
                case TerrainType::LUMINOUS_ORCHARDS:
                    color = raylib::Color::Lime();
                    break;
                case TerrainType::CRYSTAL_CANYONS:
                    color = raylib::Color::Purple();
                    break;
                case TerrainType::MAGNETIC_TUNDRA:
                    color = raylib::Color::SkyBlue();
                    break;
            }
            DrawCube(vpos, 1.0f, 1.0f, 1.0f, color);

            if (pos->x == _hoveredX && pos->y == _hoveredZ) {
                _renderHoverEffect(pos->x, pos->y);
            }
        }
    }
}

void RenderSystem::_renderInhabitants(World& w) {
    auto orientationStorage = w.get_storage<Orientation>();
    if (!orientationStorage) {
        return;
    }

    auto& am = AssetManager::getInstance();
    raylib::Model& robot = am.getModel("robot");
    float scale = 0.1f;

    // Get bounding box to center the model
    BoundingBox box = robot.GetBoundingBox();
    raylib::Vector3 center = {(box.min.x + box.max.x) / 2.0f,
                              box.min.y, // Ground it on its bottom
                              (box.min.z + box.max.z) / 2.0f};

    for (auto const& [entity, orientationPtr] : *orientationStorage) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            float rotation = 0.0f;
            switch (orientationPtr->current_direction) {
                case Orientation::N:
                    rotation = 180.0f;
                    break;
                case Orientation::E:
                    rotation = 90.0f;
                    break;
                case Orientation::S:
                    rotation = 0.0f;
                    break;
                case Orientation::W:
                    rotation = 270.0f;
                    break;
            }

            // Calculate position: center the model on the tile and nudge it 'forward' (+0.2)
            raylib::Vector3 vpos((float)pos->x + 0.2f - (center.x * scale),
                                 2.01f - (center.y * scale),
                                 (float)pos->y + 0.2f - (center.z * scale));

            robot.Draw(vpos, {0, 1, 0}, rotation, {scale, scale, scale}, WHITE);
        }
    }
}

void RenderSystem::_renderResources(World& w) {
    auto terrainStorage = w.get_storage<TerrainType>();
    if (!terrainStorage) {
        return;
    }

    raylib::Vector3 cameraTarget = _camera.target;
    auto& am = AssetManager::getInstance();
    raylib::Model& chicken = am.getModel("chicken");
    float chickenScale = 0.17f;

    BoundingBox cBox = chicken.GetBoundingBox();
    raylib::Vector3 cCenter = {(cBox.min.x + cBox.max.x) / 2.0f, cBox.min.y,
                               (cBox.min.z + cBox.max.z) / 2.0f};

    for (auto const& [entity, type] : *terrainStorage) {
        auto pos = w.get_component<Position>(entity);
        auto inv = w.get_component<Inventory>(entity);
        if (pos && inv) {
            // Only render resources if tile is close to camera
            float dist = std::sqrt(std::pow(pos->x - cameraTarget.x, 2) +
                                   std::pow(pos->y - cameraTarget.z, 2));
            if (dist > 40.0f) {
                continue;
            }

            float yBase = 2.01f;

            // Use chicken model for food, offset slightly from center
            if (inv->food > 0) {
                raylib::Vector3 cPos((float)pos->x - 0.2f - (cCenter.x * chickenScale),
                                     yBase - (cCenter.y * chickenScale),
                                     (float)pos->y - 0.2f - (cCenter.z * chickenScale));
                chicken.Draw(cPos, chickenScale, WHITE);
            }

            float startX = (float)pos->x - 0.3f;
            float startZ = (float)pos->y - 0.3f;
            float size = 0.12f;

            // Use DrawCube instead of DrawSphere for resources - much faster
            if (inv->linemate > 0) {
                DrawCube({startX, yBase + size / 2, startZ}, size, size, size, WHITE);
            }
            if (inv->deraumere > 0) {
                DrawCube({startX + 0.2f, yBase + size / 2, startZ}, size, size, size, SKYBLUE);
            }
            if (inv->sibur > 0) {
                DrawCube({startX + 0.4f, yBase + size / 2, startZ}, size, size, size, DARKBLUE);
            }
            if (inv->mendiane > 0) {
                DrawCube({startX, yBase + size / 2, startZ + 0.2f}, size, size, size, PINK);
            }
            if (inv->phiras > 0) {
                DrawCube({startX + 0.2f, yBase + size / 2, startZ + 0.2f}, size, size, size,
                         DARKPURPLE);
            }
            if (inv->thystame > 0) {
                DrawCube({startX + 0.4f, yBase + size / 2, startZ + 0.2f}, size, size, size, GOLD);
            }
        }
    }
}

void RenderSystem::_renderLandmarks(World& w) {
    // Placeholder for monoliths, fissures, etc.
}

void RenderSystem::_renderHoverEffect(int x, int z) {
    raylib::Color hCol = raylib::Color::White();
    float wH = 0.15f;
    float wT = 0.05f;
    float yB = 2.0f;

    DrawCube({(float)x, yB + wH / 2, (float)z + 0.5f}, 1.0f + wT, wH, wT, hCol);
    DrawCube({(float)x, yB + wH / 2, (float)z - 0.5f}, 1.0f + wT, wH, wT, hCol);
    DrawCube({(float)x + 0.5f, yB + wH / 2, (float)z}, wT, wH, 1.0f + wT, hCol);
    DrawCube({(float)x - 0.5f, yB + wH / 2, (float)z}, wT, wH, 1.0f + wT, hCol);
}

} // namespace zappy
