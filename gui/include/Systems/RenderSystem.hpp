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
     * @brief Main update loop for the rendering system.
     *
     * Orchestrates input handling, state updates, and the full rendering pipeline
     * (3D world followed by 2D UI).
     *
     * @param r The registry containing all entities and components to be rendered.
     */
    void update(Register& r);

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

  private:
    /**
     * @brief Lazily loads textures and models once the OpenGL context is ready.
     */
    void _lazyLoadAssets();

    /**
     * @brief Processes user input for camera movement (WASD), rotation (QE/RF), and zoom.
     */
    void _handleInput();

    /**
     * @brief Renders the terrain tiles based on their types and positions.
     * @param r The registry to fetch terrain data from.
     */
    void _renderTerrain(Register& r);

    /**
     * @brief Renders the inhabitants (bots) in the world.
     * @param r The registry to fetch bot data from.
     */
    void _renderInhabitants(Register& r);

    /**
     * @brief Draws a visual highlight (muro) around a specific tile.
     * @param x The X coordinate of the tile.
     * @param z The Z coordinate of the tile.
     */
    void _renderHoverEffect(int x, int z);

    /**
     * @brief Renders 2D UI elements, including the custom mouse cursor.
     */
    void _renderUI();

    /**
     * @brief Updates the internal state of which tile is currently under the mouse cursor.
     * Uses raycasting from the screen to the world ground plane.
     */
    void _updateHoverState();

    raylib::Camera3D _camera; ///< The 3D camera used for world rendering.

    raylib::Texture2D _mouseTex;        ///< Standard mouse cursor texture.
    raylib::Texture2D _mousePressedTex; ///< Pressed state mouse cursor texture.

    int _hoveredX = -999999; ///< Current hovered tile X coordinate.
    int _hoveredZ = -999999; ///< Current hovered tile Z coordinate.
};
} // namespace zappy

#endif // ZAPPY_GUI_RENDERSYSTEM_HPP
