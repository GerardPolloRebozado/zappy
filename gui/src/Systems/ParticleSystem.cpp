#include "Systems/ParticleSystem.hpp"
#include "Components/ComponentIncantationEffect.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Graphics/AssetManager.hpp"
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
                ::PlaySound(AssetManager::getInstance().getSound("egg_layed"));
            }
            eventsToRemove.push_back(entity);
        }
        for (const auto& e : eventsToRemove) {
            w.remove_component<EventEggHatched>(e);
        }
    }

    auto incantationStartStorage = w.get_storage<EventIncantationStart>();
    if (incantationStartStorage) {
        std::vector<Entity> startEventsToRemove;
        for (auto const& [entity, event] : *incantationStartStorage) {
            auto posComponent = w.get_component<Position>(entity);
            if (posComponent) {
                int x = posComponent->x;
                int y = posComponent->y;

                for (Entity player : event->participants) {
                    auto anim = w.get_component<Animation>(player);
                    if (anim) {
                        anim->currentAnim = "inhabitant_special_Skeletons_Awaken_Standing";
                        anim->loop = true;
                    }
                }

                auto effectEntity = w.spawn();
                ComponentParticleEmitter colEmitter;
                colEmitter.isPlaying = true;
                colEmitter.loop = true;
                colEmitter.emitRate = 60.0f;
                colEmitter.minLifetime = 2.0f;
                colEmitter.maxLifetime = 4.0f;
                colEmitter.minSize = 0.05f;
                colEmitter.maxSize = 0.1f;
                colEmitter.startColor = raylib::Color(255, 255, 255, 255);
                colEmitter.endColor = raylib::Color(255, 200, 100, 0);
                colEmitter.offset = raylib::Vector3(0.0f, 2.0f, 0.0f);
                colEmitter.spawnVolumeMin = raylib::Vector3(-0.4f, 0.0f, -0.4f);
                colEmitter.spawnVolumeMax = raylib::Vector3(0.4f, 10.0f, 0.4f);
                colEmitter.minVelocity = raylib::Vector3(-0.2f, -0.5f, -0.2f);
                colEmitter.maxVelocity = raylib::Vector3(0.2f, 0.5f, 0.2f);

                w.add_component(effectEntity, colEmitter);
                w.add_component(effectEntity,
                                ComponentIncantationEffect{x, y, event->participants});
                w.add_component(effectEntity, Position{x, y});

                auto baseEntity = w.spawn();
                ComponentParticleEmitter baseEmitter;
                baseEmitter.isPlaying = true;
                baseEmitter.loop = true;
                baseEmitter.emitRate = 30.0f;
                baseEmitter.minLifetime = 1.0f;
                baseEmitter.maxLifetime = 2.0f;
                baseEmitter.minSize = 0.1f;
                baseEmitter.maxSize = 0.2f;
                baseEmitter.startColor = raylib::Color(255, 220, 100, 150);
                baseEmitter.endColor = raylib::Color(200, 50, 255, 0);
                baseEmitter.offset = raylib::Vector3(0.0f, 2.0f, 0.0f);
                baseEmitter.spawnVolumeMin = raylib::Vector3(-0.5f, 0.0f, -0.5f);
                baseEmitter.spawnVolumeMax = raylib::Vector3(0.5f, 1.5f, 0.5f);
                baseEmitter.minVelocity = raylib::Vector3(-0.5f, 0.5f, -0.5f);
                baseEmitter.maxVelocity = raylib::Vector3(0.5f, 2.5f, 0.5f);

                w.add_component(baseEntity, baseEmitter);
                w.add_component(baseEntity, ComponentIncantationEffect{x, y, {}});
                w.add_component(baseEntity, Position{x, y});
            }
            startEventsToRemove.push_back(entity);
        }
        for (const auto& e : startEventsToRemove) {
            w.remove_component<EventIncantationStart>(e);
        }
    }

    auto incantationEndStorage = w.get_storage<EventIncantationEnd>();
    if (incantationEndStorage) {
        std::vector<Entity> endEventsToRemove;
        for (auto const& [entity, event] : *incantationEndStorage) {
            auto posComponent = w.get_component<Position>(entity);
            if (posComponent) {
                int x = posComponent->x;
                int y = posComponent->y;
                int result = event->result;

                auto effectStorage = w.get_storage<ComponentIncantationEffect>();
                std::vector<Entity> effectsToRemove;

                if (effectStorage) {
                    for (const auto& [effEntity, effect] : *effectStorage) {
                        if (effect->x == x && effect->y == y) {
                            effectsToRemove.push_back(effEntity);

                            for (Entity player : effect->participants) {
                                auto anim = w.get_component<Animation>(player);
                                if (anim) {
                                    anim->currentAnim = "inhabitant_general_Idle_A";
                                }
                            }

                            auto burstEntity = w.spawn();
                            ComponentParticleEmitter burst;
                            burst.isPlaying = true;
                            burst.loop = false;
                            burst.emitRate = 200.0f;
                            burst.duration = 0.5f;
                            burst.minLifetime = 0.5f;
                            burst.maxLifetime = 1.0f;
                            burst.minSize = 0.1f;
                            burst.maxSize = 0.3f;
                            if (result == 1) {
                                burst.startColor = raylib::Color(50, 255, 50, 255);
                                burst.endColor = raylib::Color(200, 255, 200, 0);
                            } else {
                                burst.startColor = raylib::Color(255, 50, 50, 255);
                                burst.endColor = raylib::Color(255, 200, 200, 0);
                            }
                            burst.offset = raylib::Vector3(0.0f, 0.5f, 0.0f);
                            burst.spawnVolumeMin = raylib::Vector3(-0.5f, 0.0f, -0.5f);
                            burst.spawnVolumeMax = raylib::Vector3(0.5f, 1.0f, 0.5f);
                            burst.minVelocity = raylib::Vector3(-2.0f, 1.0f, -2.0f);
                            burst.maxVelocity = raylib::Vector3(2.0f, 4.0f, 2.0f);

                            w.add_component(burstEntity, burst);
                            w.add_component(burstEntity, Position{x, y});
                        }
                    }
                }

                for (Entity e : effectsToRemove) {
                    w.despawn(e);
                }
            }
            endEventsToRemove.push_back(entity);
        }
        for (const auto& e : endEventsToRemove) {
            w.remove_component<EventIncantationEnd>(e);
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
                    p.position.x += randf(emitter.spawnVolumeMin.x, emitter.spawnVolumeMax.x);
                    p.position.y += randf(emitter.spawnVolumeMin.y, emitter.spawnVolumeMax.y);
                    p.position.z += randf(emitter.spawnVolumeMin.z, emitter.spawnVolumeMax.z);
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
