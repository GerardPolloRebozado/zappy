#ifndef ZAPPY_PARTICLESYSTEM_HPP
#define ZAPPY_PARTICLESYSTEM_HPP

#include "ECS/World.hpp"

namespace zappy {

class ParticleSystem {
  public:
    void update(World& w, float dt);

  private:
    void _handleEggs(World& w);
    void _handleDeaths(World& w);
    void _handleIncantationStart(World& w);
    void _handleIncantationEnd(World& w);
    void _handleJumps(World &w);
    void _updateEmitters(World& w, float dt);
};

} // namespace zappy

#endif // ZAPPY_PARTICLESYSTEM_HPP
