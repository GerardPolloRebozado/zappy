#ifndef ZAPPY_BACKGROUNDSYSTEM_HPP
#define ZAPPY_BACKGROUNDSYSTEM_HPP

#include "ECS/World.hpp"

namespace zappy {

/**
 * To use the music you need to add `ComponentMusic` to an entity, and precise the `.mp3` file of
 * the music The music is then played each tick and removed when finished When the `loop` attribute
 * is set to true, the song will start again when finished.
 */
class BackgroundSystem {
  public:
    void update(World& w, float dt);
};

} // namespace zappy

#endif // ZAPPY_BACKGROUNDSYSTEM_HPP
