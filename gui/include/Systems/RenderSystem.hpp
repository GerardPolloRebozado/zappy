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

    void update(Register& r) {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        camera.BeginMode();

        // Draw Grid for reference
        DrawGrid(100, 1.0f);

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
                DrawCubeWires(vpos, 1.0f, 1.0f, 1.0f, raylib::Color::LightGray());
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
    }

    raylib::Camera3D camera;
};
} // namespace zappy

#endif
