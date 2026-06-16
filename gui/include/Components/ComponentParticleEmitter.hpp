#ifndef ZAPPY_COMPONENTPARTICLEEMITTER_HPP
#define ZAPPY_COMPONENTPARTICLEEMITTER_HPP

#include "Graphics/Particle.hpp"
#include <string>
#include <vector>

namespace zappy {

struct ComponentParticleEmitter {
    // --- Playback State ---
    bool isPlaying = true;
    bool loop = false;     // True for constant effects, false for instant effects
    float duration = 1.0f; // How long the emitter spawns particles
    float timeElapsed = 0.0f;
    std::string spriteName = ""; // If empty, renders default colored quads

    // --- Emission Settings ---
    float emitRate = 50.0f; // Particles per second
    float emitAccumulator = 0.0f;

    // --- Particle Template (Ranges for Randomization) ---
    raylib::Vector3 offset = {0, 0, 0};
    float minLifetime = 0.5f, maxLifetime = 1.0f;
    float minSize = 0.05f, maxSize = 0.15f;
    raylib::Vector3 minVelocity = {-1, 0, -1};
    raylib::Vector3 maxVelocity = {1, 2, 1};
    raylib::Color startColor = raylib::Color::Orange();
    raylib::Color endColor = raylib::Color::Blank(); // Fade to transparent

    // --- Runtime Data ---
    std::vector<Particle> particles;
};

} // namespace zappy

#endif // ZAPPY_COMPONENTPARTICLEEMITTER_HPP
