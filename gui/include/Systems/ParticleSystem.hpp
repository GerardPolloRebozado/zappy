#ifndef ZAPPY_PARTICLESYSTEM_HPP
#define ZAPPY_PARTICLESYSTEM_HPP

#include "ECS/World.hpp"

namespace zappy {

class ParticleSystem {
  public:
    void update(World& w, float dt);
};

} // namespace zappy

#endif // ZAPPY_PARTICLESYSTEM_HPP
