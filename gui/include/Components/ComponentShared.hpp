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

enum class ResourceType {
    FOOD = 0,
    LINEMATE = 1,
    DERAUMERE = 2,
    SIBUR = 3,
    MENDIANE = 4,
    PHIRAS = 5,
    THYSTAME = 6
};

struct Inventory {
    int food;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
    float exactHp = -1.0f;
    float maxHp = 1260.0f;
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
    bool finished = false;
};

/**
 * @struct MovementInterpolation2D
 * @brief Component that decouples the logical ECS position from the rendering position.
 */
struct MovementInterpolation2D {
    float visualX = -1.0f; ///< The smooth visual X coordinate of the entity.
    float visualY = -1.0f; ///< The smooth visual Y coordinate of the entity.
    bool isMoving =
        false; ///< True if the visual position is currently moving towards the logical position.
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
} // namespace zappy

#endif // ZAPPY_GUI_COMPONENTSHARED_HPP
