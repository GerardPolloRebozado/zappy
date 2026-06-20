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

void ParticleSystem::update(World& w, float dt) {
    _handleEggs(w);
    _handleDeaths(w);
    _handleIncantationStart(w);
    _handleIncantationEnd(w);
    _updateEmitters(w, dt);
}

} // namespace zappy
