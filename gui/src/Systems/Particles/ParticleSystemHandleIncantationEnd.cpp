#include "Components/ComponentIncantationEffect.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Graphics/AssetManager.hpp"
#include "Systems/ParticleSystem.hpp"
#include <cstdlib>

namespace zappy {

void ParticleSystem::_handleIncantationEnd(World& w) {
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
                            burst.emitRate = 500.0f;
                            burst.duration = 0.3f;
                            burst.minLifetime = 0.5f;
                            burst.maxLifetime = 1.2f;
                            burst.minSize = 0.03f;
                            burst.maxSize = 0.08f;
                            if (result == 1) {
                                // Success: Bright cyan/white timey explosion upwards
                                burst.startColor = raylib::Color(200, 255, 255, 255);
                                burst.endColor = raylib::Color(150, 200, 255, 0);
                                burst.minVelocity = raylib::Vector3(-3.0f, 2.0f, -3.0f);
                                burst.maxVelocity = raylib::Vector3(3.0f, 6.0f, 3.0f);
                            } else {
                                // Failure: Darker blue shattered timey fizzle downwards
                                burst.startColor = raylib::Color(100, 150, 255, 255);
                                burst.endColor = raylib::Color(50, 50, 150, 0);
                                burst.minVelocity = raylib::Vector3(-1.0f, -1.0f, -1.0f);
                                burst.maxVelocity = raylib::Vector3(1.0f, 0.5f, 1.0f);
                            }
                            burst.offset = raylib::Vector3(0.0f, 0.5f, 0.0f);
                            burst.spawnVolumeMin = raylib::Vector3(-0.5f, 0.0f, -0.5f);
                            burst.spawnVolumeMax = raylib::Vector3(0.5f, 1.0f, 0.5f);

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
}

} // namespace zappy
