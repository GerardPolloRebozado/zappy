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
    _camera.position = (raylib::Vector3){(float)width / 2.0f, (float)std::max(width, height),
                                         (float)height + 10.0f};
}

void RenderSystem::update(World& w) {
    _lazyLoadAssets();
    _handleInput();
    _updateHoverState();

    _camera.BeginMode();
    _renderTerrain(w);
    _renderInhabitants(w);
    _camera.EndMode();

    _renderUI();
}

void RenderSystem::_lazyLoadAssets() {
    if (_mouseTex.id == 0) {
        try {
            _mouseTex.Load("assets/mouse.png");
            _mousePressedTex.Load("assets/mouse_pressed.png");
            HideCursor();
        } catch (const raylib::RaylibException& e) {
            std::cerr << "RenderSystem: Failed to load UI assets: " << e.what() << std::endl;
        }
    }
}

void RenderSystem::_handleInput() {
    float dt = GetFrameTime();
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

    for (auto const& [entity, type] : *terrainStorage) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
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
    auto botStorage = w.get_storage<InhabitantData>();
    if (!botStorage) {
        return;
    }

    for (auto const& [entity, bot] : *botStorage) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            raylib::Vector3 vpos(pos->x, 2.5f, pos->y);
            DrawSphere(vpos, 0.3f, RED);
        }
    }
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

void RenderSystem::_renderUI() {
    raylib::Texture2D& tex = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? _mousePressedTex : _mouseTex;
    if (tex.id != 0) {
        DrawTextureEx(tex, {(float)GetMouseX(), (float)GetMouseY()}, 0.0f, 3.0f, WHITE);
    }
}

} // namespace zappy
