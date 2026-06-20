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

void ParticleSystem::_handleIncantationStart(World& w) {
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
                        anim->currentAnim = "inhabitant_simulation_Push_Ups";
                        anim->loop = true;
                    }

                    auto move = w.get_component<MovementInterpolation2D>(player);
                    if (move) {
                        auto feetEntity = w.spawn();
                        ComponentParticleEmitter feetEmitter;
                        feetEmitter.isPlaying = true;
                        feetEmitter.loop = true;
                        feetEmitter.emitRate = 30.0f;
                        feetEmitter.minLifetime = 0.5f;
                        feetEmitter.maxLifetime = 1.0f;
                        feetEmitter.minSize = 0.02f;
                        feetEmitter.maxSize = 0.04f;
                        feetEmitter.startColor = raylib::Color(255, 255, 255, 255);
                        feetEmitter.endColor = raylib::Color(150, 200, 255, 0);
                        // basePos will be (x, 2.0f, y). We offset it to visual position:
                        feetEmitter.offset =
                            raylib::Vector3(move->visualX - x, 0.0f, move->visualY - y);
                        feetEmitter.spawnVolumeMin = raylib::Vector3(-0.3f, 0.0f, -0.3f);
                        feetEmitter.spawnVolumeMax = raylib::Vector3(0.3f, 0.2f, 0.3f);
                        feetEmitter.minVelocity = raylib::Vector3(-0.2f, 0.5f, -0.2f);
                        feetEmitter.maxVelocity = raylib::Vector3(0.2f, 1.0f, 0.2f);

                        w.add_component(feetEntity, feetEmitter);
                        w.add_component(feetEntity, ComponentIncantationEffect{x, y, {}});
                        w.add_component(feetEntity, Position{x, y});
                    }
                }

                auto effectEntity = w.spawn();
                ComponentParticleEmitter colEmitter;
                colEmitter.isPlaying = true;
                colEmitter.loop = true;
                colEmitter.emitRate = 60.0f;
                colEmitter.minLifetime = 2.0f;
                colEmitter.maxLifetime = 4.0f;
                colEmitter.minSize = 0.02f;
                colEmitter.maxSize = 0.05f;
                colEmitter.startColor = raylib::Color(255, 255, 255, 255);
                colEmitter.endColor = raylib::Color(150, 200, 255, 0);
                colEmitter.offset = raylib::Vector3(0.0f, 2.0f, 0.0f);
                colEmitter.spawnRadius = 0.6f;
                colEmitter.spawnVolumeMin = raylib::Vector3(0.0f, 0.0f, 0.0f);
                colEmitter.spawnVolumeMax = raylib::Vector3(0.0f, 5.0f, 0.0f);
                colEmitter.minVelocity = raylib::Vector3(-0.2f, -0.2f, -0.2f);
                colEmitter.maxVelocity = raylib::Vector3(0.2f, 0.5f, 0.2f);

                w.add_component(effectEntity, colEmitter);
                w.add_component(effectEntity,
                                ComponentIncantationEffect{x, y, event->participants});
                w.add_component(effectEntity, Position{x, y});
            }
            startEventsToRemove.push_back(entity);
        }
        for (const auto& e : startEventsToRemove) {
            w.remove_component<EventIncantationStart>(e);
        }
    }
}

} // namespace zappy
