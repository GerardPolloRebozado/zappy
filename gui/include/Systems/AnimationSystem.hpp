#ifndef ZAPPY_GUI_ANIMATIONSYSTEM_HPP
#define ZAPPY_GUI_ANIMATIONSYSTEM_HPP

#include "Components/ComponentShared.hpp"
#include "ECS/World.hpp"
#include "Graphics/AssetManager.hpp"

namespace zappy {
/**
 * @class AnimationSystem
 * @brief System responsible for processing entity animations and movement interpolation.
 *
 * This system iterates over entities with Animation and MovementInterpolation components
 * to advance their visual positions and animation frames smoothly based on the server time
 * frequency.
 */
class AnimationSystem {
  public:
    AnimationSystem() = default;
    ~AnimationSystem() = default;

    /**
     * @brief Update the movement interpolation and animation frames for all relevant entities.
     * @param w The game world containing the entities.
     */
    void update(World& w);

  private:
    void _update2DMovement(World& w, float freq);
    void _update3DMovement(World& w, float freq);
    void _updateAnimations(World& w);
};
} // namespace zappy

#endif // ZAPPY_GUI_ANIMATIONSYSTEM_HPP
