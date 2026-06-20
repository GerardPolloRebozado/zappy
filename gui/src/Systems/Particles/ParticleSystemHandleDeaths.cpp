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

void ParticleSystem::_handleDeaths(World& w) {
    auto eventDeathStorage = w.get_storage<EventDeath>();
    if (eventDeathStorage) {
        std::vector<Entity> eventDeathToRemove;
        for (auto const& [entity, event] : *eventDeathStorage) {
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
                emitter.colorPalette = {raylib::Color::Gray(), raylib::Color::Black(),
                                        raylib::Color::White()};
                w.add_component(entity, emitter);
                ::PlaySound(AssetManager::getInstance().getSound("death"));

                auto anim = w.get_component<Animation>(entity);
                if (anim) {
                    anim->currentAnim = "inhabitant_general_Death_A";
                    anim->loop = false;
                    anim->currentFrame = 0.0f;
                }

                auto move = w.get_component<MovementInterpolation2D>(entity);
                if (move) {
                    move->isMoving = false;
                }
            }
            eventDeathToRemove.push_back(entity);
        }
        for (const auto& e : eventDeathToRemove) {
            w.remove_component<EventDeath>(e);
        }
    }
}

} // namespace zappy
