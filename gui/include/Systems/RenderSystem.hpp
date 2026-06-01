/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** RenderSystem.hpp
*/

#ifndef ZAPPY_GUI_RENDERSYSTEM_HPP
#define ZAPPY_GUI_RENDERSYSTEM_HPP

#include "../ECS/Register.hpp"
#include <map>
#include <raylib-cpp.hpp>
#include <raymath.h>
#include <string>
#include <vector>

namespace zappy {

class RenderSystem {
  public:
    RenderSystem();
    ~RenderSystem() = default;

    /**
     * @brief Main update loop for the rendering system.
     * @param r The registry containing all entities and components.
     */
    void update(Register& r);

    /**
     * @brief Centers the camera on the map.
     */
    void centerCamera(int width, int height);

    /**
     * @brief Returns the coordinates of the currently hovered tile.
     * Useful for future interaction systems.
     */
    std::pair<int, int> getHoveredTile() const { return {_hoveredX, _hoveredZ}; }

  private:
    // Lifecycle & Assets
    void _lazyLoadAssets();
    void _handleInput();

    // Rendering Phases
    void _renderTerrain(Register& r);
    void _renderInhabitants(Register& r);
    void _renderHoverEffect(int x, int z);
    void _renderUI();

    // Helper: Camera calculation
    void _updateHoverState();

    raylib::Camera3D _camera;

    // UI Assets
    raylib::Texture2D _mouseTex;
    raylib::Texture2D _mousePressedTex;

    // Hover State
    int _hoveredX = -999999;
    int _hoveredZ = -999999;

    // To be used in future for voxel models / more complex rendering
    // std::map<TerrainType::Type, raylib::Model> _tileModels;
    // std::vector<raylib::Model> _botModels;
};

} // namespace zappy

#endif // ZAPPY_GUI_RENDERSYSTEM_HPP
