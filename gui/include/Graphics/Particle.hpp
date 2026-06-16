#ifndef ZAPPY_PARTICLE_HPP
#define ZAPPY_PARTICLE_HPP

#include <raylib-cpp.hpp>

namespace zappy {

struct Particle {
    raylib::Vector3 position;
    raylib::Vector3 velocity;
    raylib::Color startColor;
    raylib::Color endColor;
    float startSize;
    float endSize;
    float lifetime;
    float lifeRemaining;
};

} // namespace zappy

#endif // ZAPPY_PARTICLE_HPP
