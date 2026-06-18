#include "Systems/ParticleSystem.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include <cstdlib>

namespace zappy {

static float randf(float min, float max) {
    return min + (max - min) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

void ParticleSystem::update(World& w, float dt) {
    auto eventStorage = w.get_storage<EventEggHatched>();
    if (eventStorage) {
        std::vector<Entity> eventsToRemove;
        for (auto const& [entity, event] : *eventStorage) {
            auto posComponent = w.get_component<Position>(entity);
            if (posComponent) {
                ComponentParticleEmitter emitter;
                emitter.isPlaying = true;
                emitter.loop = false;
                emitter.duration = 0.1f;
                emitter.emitRate = 500.0f;
                emitter.minLifetime = 0.5f;
                emitter.maxLifetime = 1.5f;
                emitter.minSize = 0.05f;
                emitter.maxSize = 0.15f;
                emitter.minVelocity = raylib::Vector3(-3.0f, 4.0f, -3.0f);
                emitter.maxVelocity = raylib::Vector3(3.0f, 8.0f, 3.0f);
                emitter.colorPalette = {raylib::Color::Red(),     raylib::Color::Green(),
                                        raylib::Color::Blue(),    raylib::Color::Yellow(),
                                        raylib::Color::Magenta(), raylib::Color::SkyBlue(),
                                        raylib::Color::Orange()};

                w.add_component(entity, emitter);
                w.add_component(entity, std::make_shared<ComponentMusic>(
                                            std::string("assets/sounds/effect/egg_layed.mp3")));
            }
            eventsToRemove.push_back(entity);
        }
        for (const auto& e : eventsToRemove) {
            w.remove_component<EventEggHatched>(e);
        }
    }

    auto storage = w.get_storage<ComponentParticleEmitter>();
    if (!storage) {
        return;
    }

    std::vector<Entity> toRemove;

    for (auto& [entity, emitterPtr] : *storage) {
        auto& emitter = *emitterPtr;

        raylib::Vector3 basePos(0.0f, 0.0f, 0.0f);
        auto posComponent = w.get_component<Position>(entity);
        if (posComponent) {
            basePos = raylib::Vector3(static_cast<float>(posComponent->x), 2.0f,
                                      static_cast<float>(posComponent->y));
        }

        if (emitter.isPlaying) {
            emitter.timeElapsed += dt;

            if (emitter.loop || emitter.timeElapsed <= emitter.duration) {
                emitter.emitAccumulator += dt * emitter.emitRate;

                while (emitter.emitAccumulator >= 1.0f) {
                    emitter.emitAccumulator -= 1.0f;

                    Particle p;
                    p.position = basePos + emitter.offset;
                    p.velocity =
                        raylib::Vector3(randf(emitter.minVelocity.x, emitter.maxVelocity.x),
                                        randf(emitter.minVelocity.y, emitter.maxVelocity.y),
                                        randf(emitter.minVelocity.z, emitter.maxVelocity.z));

                    if (!emitter.colorPalette.empty()) {
                        int colorIdx = rand() % emitter.colorPalette.size();
                        p.startColor = emitter.colorPalette[colorIdx];
                        p.endColor = p.startColor;
                        p.endColor.a = 0; // Fade to transparent
                    } else {
                        p.startColor = emitter.startColor;
                        p.endColor = emitter.endColor;
                    }

                    p.startSize = randf(emitter.minSize, emitter.maxSize);
                    p.endSize = p.startSize * randf(0.1f, 0.5f); // end smaller by default
                    p.lifetime = randf(emitter.minLifetime, emitter.maxLifetime);
                    p.lifeRemaining = p.lifetime;

                    emitter.particles.push_back(p);
                }
            } else if (!emitter.loop && emitter.timeElapsed > emitter.duration) {
                emitter.isPlaying = false;
            }
        }

        for (auto it = emitter.particles.begin(); it != emitter.particles.end();) {
            it->position += it->velocity * dt;
            it->lifeRemaining -= dt;

            if (it->lifeRemaining <= 0.0f) {
                it = emitter.particles.erase(it);
            } else {
                ++it;
            }
        }

        if (!emitter.loop && !emitter.isPlaying && emitter.particles.empty()) {
            toRemove.push_back(entity);
        }
    }

    for (const auto& entity : toRemove) {
        w.despawn(entity);
    }
}

} // namespace zappy
