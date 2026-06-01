/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** RenderSystem.hpp
*/
#ifndef ZAPPY_GUI_RENDERSYSTEM_HPP
#define ZAPPY_GUI_RENDERSYSTEM_HPP

#include "../ECS/Register.hpp"
#include <raylib-cpp.hpp>
#include <raymath.h>

namespace zappy {
class RenderSystem {
  public:
    RenderSystem() {
        camera.position = (raylib::Vector3){20.0f, 20.0f, 20.0f};
        camera.target = (raylib::Vector3){0.0f, 0.0f, 0.0f};
        camera.up = (raylib::Vector3){0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }

    void centerCamera(int width, int height) {
        camera.target = (raylib::Vector3){(float)width / 2.0f, 0.0f, (float)height / 2.0f};
        camera.position = (raylib::Vector3){(float)width / 2.0f, (float)std::max(width, height),
                                            (float)height + 10.0f};
    }

    void handleInput() {
        float dt = GetFrameTime();
        float moveSpeed = 20.0f * dt;
        float rotateSpeed = 2.0f * dt;

        raylib::Vector3 forward = (raylib::Vector3)camera.target - (raylib::Vector3)camera.position;
        raylib::Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

        // Rotation
        if (IsKeyDown(KEY_Q)) {
            raylib::Vector3 relPos =
                (raylib::Vector3)camera.position - (raylib::Vector3)camera.target;
            relPos = Vector3RotateByAxisAngle(relPos, {0, 1, 0}, rotateSpeed);
            camera.position = (raylib::Vector3)camera.target + relPos;
        }
        if (IsKeyDown(KEY_E)) {
            raylib::Vector3 relPos =
                (raylib::Vector3)camera.position - (raylib::Vector3)camera.target;
            relPos = Vector3RotateByAxisAngle(relPos, {0, 1, 0}, -rotateSpeed);
            camera.position = (raylib::Vector3)camera.target + relPos;
        }

        if (IsKeyDown(KEY_R)) { // Tilt up (more top-down)
            raylib::Vector3 relPos =
                (raylib::Vector3)camera.position - (raylib::Vector3)camera.target;
            raylib::Vector3 nextRelPos = Vector3RotateByAxisAngle(relPos, right, -rotateSpeed);
            float angle = Vector3Angle(nextRelPos, {0, 1, 0});
            if (angle > 0.1f && angle < 1.5f) {
                camera.position = (raylib::Vector3)camera.target + nextRelPos;
            }
        }
        if (IsKeyDown(KEY_F)) { // Tilt down (more horizontal)
            raylib::Vector3 relPos =
                (raylib::Vector3)camera.position - (raylib::Vector3)camera.target;
            raylib::Vector3 nextRelPos = Vector3RotateByAxisAngle(relPos, right, rotateSpeed);
            float angle = Vector3Angle(nextRelPos, {0, 1, 0});
            if (angle > 0.1f && angle < 1.5f) {
                camera.position = (raylib::Vector3)camera.target + nextRelPos;
            }
        }

        // Movement
        forward = (raylib::Vector3)camera.target - (raylib::Vector3)camera.position;
        forward.y = 0;
        forward = Vector3Normalize(forward);
        right = Vector3CrossProduct(forward, camera.up);

        if (IsKeyDown(KEY_W)) {
            camera.position = (raylib::Vector3)camera.position + forward * moveSpeed;
            camera.target = (raylib::Vector3)camera.target + forward * moveSpeed;
        }
        if (IsKeyDown(KEY_S)) {
            camera.position = (raylib::Vector3)camera.position - forward * moveSpeed;
            camera.target = (raylib::Vector3)camera.target - forward * moveSpeed;
        }
        if (IsKeyDown(KEY_A)) {
            camera.position = (raylib::Vector3)camera.position - right * moveSpeed;
            camera.target = (raylib::Vector3)camera.target - right * moveSpeed;
        }
        if (IsKeyDown(KEY_D)) {
            camera.position = (raylib::Vector3)camera.position + right * moveSpeed;
            camera.target = (raylib::Vector3)camera.target + right * moveSpeed;
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Ray ray = GetMouseRay(GetMousePosition(), camera);
            float groundY = 1.0f;
            if (ray.direction.y != 0) {
                float t = (groundY - ray.position.y) / ray.direction.y;
                if (t > 0) {
                    raylib::Vector3 mousePoint =
                        (raylib::Vector3)ray.position + (raylib::Vector3)ray.direction * t;
                    raylib::Vector3 zoomVec =
                        (mousePoint - (raylib::Vector3)camera.position) * (wheel * 0.1f);

                    // Limit zoom
                    float nextDist =
                        Vector3Distance((raylib::Vector3)camera.position + zoomVec, mousePoint);
                    if (wheel > 0 && nextDist < 2.0f) {
                        return;
                    }
                    if (wheel < 0 && nextDist > 100.0f) {
                        return;
                    }

                    camera.position = (raylib::Vector3)camera.position + zoomVec;
                    camera.target = (raylib::Vector3)camera.target + zoomVec;
                }
            }
        }
    }

    void drawMouseCursor() {
        raylib::Texture2D& tex = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? mousePressedTex : mouseTex;
        if (tex.id != 0) {
            DrawTextureEx(tex, {(float)GetMouseX(), (float)GetMouseY()}, 0.0f, 3.0f, WHITE);
        }
    }

    void update(Register& r) {
        if (mouseTex.id == 0) {
            try {
                mouseTex.Load("assets/mouse.png");
                mousePressedTex.Load("assets/mouse_pressed.png");
                HideCursor();
            } catch (const raylib::RaylibException& e) {
                // Assets might be missing, fallback is handled in drawMouseCursor
            }
        }
        handleInput();

        camera.BeginMode();

        for (auto const& [entity, type] : r._terrainTypes) {
            if (r._positions.find(entity) != r._positions.end()) {
                auto const& pos = r._positions.at(entity);
                // Raise tiles and make them taller to ensure they are clearly visible
                raylib::Vector3 vpos(pos.x, 1.5f, pos.y);

                raylib::Color color = GRAY;
                switch (type.current_type) {
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
            }
        }
        // Render inhabitants
        for (auto const& [entity, inhabitant] : r._bots) {
            if (r._positions.find(entity) != r._positions.end()) {
                auto const& pos = r._positions.at(entity);
                raylib::Vector3 vpos(pos.x, 0.5f, pos.y);
                DrawSphere(vpos, 0.3f, RED);
            }
        }

        camera.EndMode();
        drawMouseCursor();
    }

    raylib::Camera3D camera;
    raylib::Texture2D mouseTex;
    raylib::Texture2D mousePressedTex;
};
} // namespace zappy

#endif
