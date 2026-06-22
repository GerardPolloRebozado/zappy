#include "Components/ComponentTags.hpp"
#include "Systems/AnimationSystem.hpp"

namespace zappy {

void AnimationSystem::_updateAnimations(World& w) {
    auto& am = AssetManager::getInstance();
    auto animStorage = w.get_storage<Animation>();
    if (animStorage) {
        for (auto& [entity, anim] : *animStorage) {
            if (anim->currentAnim.empty()) {
                continue;
            }
            try {
                auto& modelAnim = am.getAnimation(anim->currentAnim);

                anim->currentFrame += GetFrameTime() * anim->baseFps * anim->speedMultiplier;

                if (anim->currentFrame >= modelAnim.keyframeCount) {
                    if (anim->loop) {
                        anim->currentFrame = 0.0f;
                    } else {
                        anim->finished = true;
                        anim->currentFrame = static_cast<float>(modelAnim.keyframeCount - 1);
                    }
                } else if (anim->currentFrame < 0.0f) {
                    if (anim->loop) {
                        anim->currentFrame = static_cast<float>(modelAnim.keyframeCount - 1);
                    } else {
                        anim->finished = true;
                        anim->currentFrame = 0.0f;
                    }
                }
            } catch (const std::exception& e) {
                // Animation not found, ignore
            }
        }
    }
}

} // namespace zappy
