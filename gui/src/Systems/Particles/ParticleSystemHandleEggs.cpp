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

void ParticleSystem::_handleEggs(World& w) {
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
}

} // namespace zappy
