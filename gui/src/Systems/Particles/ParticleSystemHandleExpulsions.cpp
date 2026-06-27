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

void ParticleSystem::_handleExpulsions(World& w) {
    // Expulsion
    auto eventExpulsionStorage = w.get_storage<EventExpulsion>();
    if (eventExpulsionStorage) {
        std::vector<Entity> eventExpulsionToRemove;
        for (auto const& [entity, event] : *eventExpulsionStorage) {
            auto posComponent = w.get_component<Position>(entity);
            if (posComponent) {
                raylib::Color teamColor = raylib::Color::Magenta();
                auto team = w.get_component<TeamName>(entity);
                if (team) {
                    teamColor = team->_color;
                }

                unsigned char r2 =
                    static_cast<unsigned char>(std::min(teamColor.r + 80, 255));
                unsigned char g2 =
                    static_cast<unsigned char>(std::min(teamColor.g + 80, 255));
                unsigned char b2 =
                    static_cast<unsigned char>(std::min(teamColor.b + 80, 255));
                raylib::Color lighterColor(r2, g2, b2, 255);

                auto firstEmitterEntity = w.spawn();
                ComponentParticleEmitter firstEmitter;
                firstEmitter.isPlaying = true;
                firstEmitter.loop = false;
                firstEmitter.duration = 0.1f;
                firstEmitter.emitRate = 1000.0f;
                firstEmitter.minLifetime = 0.5f;
                firstEmitter.maxLifetime = 0.5f;
                firstEmitter.minSize = 0.05f;
                firstEmitter.maxSize = 0.10f;
                firstEmitter.minVelocity = raylib::Vector3(-6.0f, 0.0f, 0.5f);
                firstEmitter.maxVelocity = raylib::Vector3(6.0f, 0.2f, -0.5f);
                firstEmitter.colorPalette = {teamColor, lighterColor,
                                        raylib::Color::Black()};
                w.add_component(firstEmitterEntity, firstEmitter);
                w.add_component(firstEmitterEntity, posComponent);

            auto secondEmitterEntity = w.spawn();
                ComponentParticleEmitter secondEmitter;
                secondEmitter.isPlaying = true;
                secondEmitter.loop = false;
                secondEmitter.duration = 0.1f;
                secondEmitter.emitRate = 1000.0f;
                secondEmitter.minLifetime = 0.5f;
                secondEmitter.maxLifetime = 0.5f;
                secondEmitter.minSize = 0.05f;
                secondEmitter.maxSize = 0.10f;
                secondEmitter.minVelocity = raylib::Vector3(-0.5f, 0.0f, 6.0f);
                secondEmitter.maxVelocity = raylib::Vector3(0.5f, 0.2f, -6.0f);
                secondEmitter.colorPalette = {teamColor, lighterColor,
                                        raylib::Color::White()};
                w.add_component(secondEmitterEntity, secondEmitter);
                w.add_component(secondEmitterEntity, posComponent);
                ::PlaySound(AssetManager::getInstance().getSound("death"));

                auto move = w.get_component<MovementInterpolation2D>(entity);
                if (move) {
                    move->isMoving = false;
                }
            }
            eventExpulsionToRemove.push_back(entity);
        }
        for (const auto& e : eventExpulsionToRemove) {
            w.remove_component<EventExpulsion>(e);
        }
    }

    // Expulsed
    auto eventExpulsedStorage = w.get_storage<EventExpulsed>();
    if (eventExpulsedStorage) {
        std::vector<Entity> eventExpulsedToRemove;
        for (auto const& [entity, event] : *eventExpulsedStorage) {
            auto anim = w.get_component<Animation>(entity);
            if (anim) {
                anim->currentAnim = "inhabitant_general_Death_B";
                anim->loop = false;
                anim->currentFrame = 0.0f;
            }
            auto move = w.get_component<MovementInterpolation2D>(entity);
            if (move) {
                move->isMoving = false;
            }
            eventExpulsedToRemove.push_back(entity);
        }
        for (const auto& e : eventExpulsedToRemove) {
            w.remove_component<EventExpulsed>(e);
        }
    }
}

} // namespace zappy
