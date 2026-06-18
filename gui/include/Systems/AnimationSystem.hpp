#ifndef ZAPPY_GUI_ANIMATIONSYSTEM_HPP
#define ZAPPY_GUI_ANIMATIONSYSTEM_HPP

#include "Components/ComponentShared.hpp"
#include "ECS/World.hpp"
#include "Graphics/AssetManager.hpp"

namespace zappy {
class AnimationSystem {
  public:
    AnimationSystem() = default;
    ~AnimationSystem() = default;

    void update(World& w);
};
} // namespace zappy

#endif // ZAPPY_GUI_ANIMATIONSYSTEM_HPP
