/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** RenderSystem.hpp
*/

#ifndef ZAPPY_GUI_RENDERSYSTEM_HPP
#define ZAPPY_GUI_RENDERSYSTEM_HPP

#include "ECS/Entity.hpp"
#include "ECS/World.hpp"
#include "Graphics/AssetManager.hpp"
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <raylib-cpp.hpp>
#include <raymath.h>
#include <string>
#include <vector>

namespace zappy {

/**
 * @class RenderSystem
 * @brief Handles all 3D and 2D rendering for the Zappy GUI.
 *
 * This system manages the camera, terrain rendering, entity visualization,
 * and UI overlays. It follows a modular pipeline to allow easy extensions
 * for tile interactions and custom effects.
 */
class RenderSystem {
  public:
    /**
     * @brief Construct a new Render System object.
     * Initializes the default camera position and orientation.
     */
    RenderSystem();

    /**
     * @brief Destroy the Render System object.
     */
    ~RenderSystem() = default;

    /**
     * @brief Logic update loop for the rendering system.
     *
     * Handles input and state updates (camera, hover state).
     * Should be called before render().
     *
     * @param w the ECS world instance
     * @param dt delta time since last frame
     */
    void update(World& w, float dt);

    /**
     * @brief Main draw loop for the rendering system.
     *
     * Orchestrates the full rendering pipeline (3D world followed by 2D UI).
     *
     * @param w the ECS world instance to query entities and components
     */
    void render(World& w);

    /**
     * @brief Centers the camera on the map based on dimensions.
     *
     * @param width The width of the map.
     * @param height The height of the map.
     */
    void centerCamera(int width, int height);

    /**
     * @brief Returns the coordinates of the currently hovered tile.
     *
     * Useful for interaction systems that need to know which tile is selected
     * by the user's mouse.
     *
     * @return std::pair<int, int> A pair containing the X and Z coordinates.
     */
    std::pair<int, int> getHoveredTile() const { return {_hoveredX, _hoveredZ}; }

    /**
     * @brief Returns the coordinates of the currently selected tile.
     * @return std::pair<int, int> A pair containing the X and Z coordinates.
     */
    std::pair<int, int> getSelectedTile() const { return {_selectedX, _selectedZ}; }

  private:
    /**
     * @brief Lazily loads textures and models once the OpenGL context is ready.
     */
    void _lazyLoadAssets();

    /**
     * @brief Processes user input for camera movement (WASD), rotation (QE/RF), and zoom.
     * @param dt delta time
     */
    void _handleInput(float dt);

    /**
     * @brief Renders the terrain tiles based on their types and positions.
     * @param w The world to fetch terrain data from.
     */
    void _renderTerrain(World& w);
    void _renderEggs(World& w);
    void _renderParticles(World& w);

    /**
     * @brief Renders the inhabitants in the world.
     * @param w The world to fetch inhabitant data from.
     */
    void _renderInhabitants(World& w);

    /**
     * @brief Renders the resources on the tiles.
     * @param w The world to fetch resource data from.
     */
    void _renderResources(World& w);
    void _renderAnimatedResources(World& w);

    /**
     * @brief Renders landmarks like monoliths and fissures.
     * @param w The world to fetch landmark data from.
     */
    void _renderLandmarks(World& w);

    /**
     * @brief Draws a visual highlight (muro) around a specific tile.
     * @param x The X coordinate of the tile.
     * @param z The Z coordinate of the tile.
     */
    void _renderHoverEffect(int x, int z);

    /**
     * @brief Updates the internal state of which tile is currently under the mouse cursor.
     * Uses raycasting from the screen to the world ground plane.
     */
    void _updateHoverState();

    /**
     * @brief Update camera position with the current following entity.
     * Only executed if its currently following one
     */
    void _renderPOV(World& w);

    raylib::Camera3D _camera; ///< The 3D camera used for world rendering.

    static constexpr int InvalidTileCoord = std::numeric_limits<int>::min();
    int _hoveredX = InvalidTileCoord;
    int _hoveredZ = InvalidTileCoord;
    int _selectedX = InvalidTileCoord;
    int _selectedZ = InvalidTileCoord;

    bool _showDebugHud = false;
    void _renderDebugHud(World& w);
};
} // namespace zappy

#endif // ZAPPY_GUI_RENDERSYSTEM_HPP
