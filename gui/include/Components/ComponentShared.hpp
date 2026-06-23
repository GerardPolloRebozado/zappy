/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentShared.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTSHARED_HPP
#define ZAPPY_GUI_COMPONENTSHARED_HPP
#include "raylib-cpp.hpp"

namespace zappy {
struct Position {
    int x;
    int y;
};

struct Size {
    int width;
    int height;
};

struct Inventory {
    int food;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
};

struct Renderable3D {
    std::string model_id;
    raylib::Color color;
};

struct TimeUnit {
    int frequency;
};

struct MapEvent {
    std::string name = "none";
    int centerX = -1;
    int centerY = -1;
    bool active = false;
};

struct ServerId {
    int id;
};
/**
 * @struct Animation
 * @brief Component that holds the state of a 3D model animation.
 */
struct Animation {
    std::string currentAnim;   ///< Identifier name of the animation loaded in the AssetManager.
    float currentFrame = 0.0f; ///< Current playback frame of the animation.
    float baseFps = 60.0f;     ///< Base frames per second the animation should run at.
    float speedMultiplier =
        1.0f;         ///< Multiplier to adjust animation speed based on game time frequency.
    bool loop = true; ///< Whether the animation should restart when reaching the end.
};

/**
 * @struct MovementInterpolation2D
 * @brief Component that decouples the logical ECS position from the rendering position.
 */
struct MovementInterpolation2D {
    float visualX = -1.0f; ///< The smooth visual X coordinate of the entity.
    float visualY = -1.0f; ///< The smooth visual Y coordinate of the entity.
    bool isMoving = false; ///< True if the visual position is currently moving towards the logical position.
};

/**
 * @struct MovementInterpolation3D
 * @brief Component that decouples the logical ECS position from the rendering position.
 */
struct MovementInterpolation3D {
    float visualX = -1.0f; ///< The smooth visual X coordinate of the entity.
    float visualY = -1.0f; ///< The smooth visual Y coordinate of the entity.
    float visualZ = -1.0f;
    bool isMoving =
        false; ///< True if the visual position is currently moving towards the logical position.
    float targetZ = 2.01f;
};

struct BackgroundParallax {
    raylib::Texture2D background = LoadTexture("assets/textures/background_1.png");
    raylib::Texture2D midground = LoadTexture("assets/textures/background_2.png");
    raylib::Texture2D foreground = LoadTexture("assets/textures/background_3.png");
    float scrollingBack = 0.0f;
    float scrollingMid = 0.0f;
    float scrollingFore = 0.0f;
};

struct CelestialObject {
    float angle = 0.0f; //< Represent the position of the object on the edge of a circle (like his path)
    float x = 0.0f; //< Position in float, to avoid using "tile position" of the normal Positon component
    float y = 0.0f;
    float size = 0.01f;
    std::string model; //< Name of the model to use
    CelestialObject(float angle, float size, std::string modelName) : angle(angle), size(size) {
        model = modelName;
    };
};
} // namespace zappy

#endif // ZAPPY_GUI_COMPONENTSHARED_HPP
