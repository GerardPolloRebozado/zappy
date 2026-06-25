#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Graphics/AssetManager.hpp"
#include "Systems/ParticleSystem.hpp"
#include <cstdlib>

namespace zappy {

void ParticleSystem::_handleBroadcast(World& w) {
    auto eventBroadcastStorage = w.get_storage<EventBroadcast>();
    if (eventBroadcastStorage) {
        std::vector<Entity> toRemoveEvent;
        for (auto& [e, ev] : *eventBroadcastStorage) {
            auto broadcasterPos = w.get_component<Position>(e);
            if (broadcasterPos) {
                float startX = static_cast<float>(broadcasterPos->x);
                float startY = static_cast<float>(broadcasterPos->y);
                float startZ = 3.5f; // Start at head height

                raylib::Color teamColor = raylib::Color::Magenta();
                auto team = w.get_component<TeamName>(e);
                if (team) {
                    teamColor = team->_color;
                }

                auto inhabitantStorage = w.get_storage<InhabitantTag>();
                if (inhabitantStorage) {
                    for (auto& [otherEntity, _] : *inhabitantStorage) {
                        if (otherEntity == e) {
                            continue; // Skip the broadcaster
                        }

                        auto otherPos = w.get_component<Position>(otherEntity);
                        if (otherPos) {
                            Entity proj = w.spawn();

                            // The destination
                            w.add_component<Position>(proj, Position{otherPos->x, otherPos->y});

                            float dx = otherPos->x - startX;
                            float dy = otherPos->y - startY;
                            float dz = 3.5f - startZ;
                            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

                            // The starting and current moving visual position
                            MovementInterpolation3D move3d;
                            move3d.visualX = startX;
                            move3d.visualZ = startZ;
                            move3d.visualY = startY;
                            move3d.targetZ = 3.5f;
                            move3d.minSpeed = dist / 0.75f; // AT MOST 0.75s travel time
                            w.add_component<MovementInterpolation3D>(proj, move3d);

                            ComponentParticleEmitter emitter;
                            emitter.isPlaying = true;
                            emitter.loop = true;      // Stay alive while moving
                            emitter.duration = 5.0f;  // Failsafe duration
                            emitter.emitRate = 80.0f; // Less particles

                            emitter.offset = raylib::Vector3(0.0f, 0.0f, 0.0f);
                            emitter.spawnRadius = 0.1f;

                            emitter.minLifetime = 0.2f;
                            emitter.maxLifetime = 0.5f;
                            emitter.minSize = 0.05f;
                            emitter.maxSize = 0.15f;

                            emitter.minVelocity = raylib::Vector3(-0.5f, -0.5f, -0.5f);
                            emitter.maxVelocity = raylib::Vector3(0.5f, 0.5f, 0.5f);

                            unsigned char r2 =
                                static_cast<unsigned char>(std::min(teamColor.r + 80, 255));
                            unsigned char g2 =
                                static_cast<unsigned char>(std::min(teamColor.g + 80, 255));
                            unsigned char b2 =
                                static_cast<unsigned char>(std::min(teamColor.b + 80, 255));
                            raylib::Color lighterColor(r2, g2, b2, 255);

                            emitter.colorPalette = {teamColor, lighterColor,
                                                    raylib::Color::RayWhite(), teamColor};

                            w.add_component<ComponentParticleEmitter>(proj, emitter);
                        }
                    }
                }
            }
            toRemoveEvent.push_back(e);
        }

        for (auto e : toRemoveEvent) {
            w.remove_component<EventBroadcast>(e);
        }
    }
}

} // namespace zappy
