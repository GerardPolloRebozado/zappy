#include "Systems/AnimationSystem.hpp"
#include "Components/ComponentTags.hpp"

namespace zappy {

void AnimationSystem::update(World& w) {
    float freq = 100.0f;
    auto timeStorage = w.get_storage<TimeUnit>();
    if (timeStorage && timeStorage->begin() != timeStorage->end()) {
        freq = static_cast<float>(timeStorage->begin()->second->frequency);
    }
    if (freq <= 0.0f) {
        freq = 1.0f;
    }

    _update2DMovement(w, freq);
    _update3DMovement(w, freq);
    _updateAnimations(w);
}

} // namespace zappy
