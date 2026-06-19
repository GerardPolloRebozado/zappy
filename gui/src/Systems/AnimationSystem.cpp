#include "Systems/AnimationSystem.hpp"
#include "Components/ComponentTags.hpp"

namespace zappy {

void AnimationSystem::update(World& w) {
    auto& am = AssetManager::getInstance();

    // Process movement interpolation
    float freq = 100.0f;
    auto timeStorage = w.get_storage<TimeUnit>();
    if (timeStorage && timeStorage->begin() != timeStorage->end()) {
        freq = static_cast<float>(timeStorage->begin()->second->frequency);
    }
    if (freq <= 0.0f) {
        freq = 1.0f;
    }

    auto moveStorage = w.get_storage<MovementInterpolation>();
    if (moveStorage) {
        for (auto& [entity, move] : *moveStorage) {
            auto pos = w.get_component<Position>(entity);
            if (!pos) {
                continue;
            }

            if (move->visualX < 0.0f) {
                move->visualX = static_cast<float>(pos->x);
                move->visualY = static_cast<float>(pos->y);
            }

            float targetX = static_cast<float>(pos->x);
            float targetY = static_cast<float>(pos->y);
            float speed = (freq / 7.0f) * GetFrameTime();

            float dx = targetX - move->visualX;
            float dy = targetY - move->visualY;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 0.05f) {
                move->isMoving = true;
                float moveX = (dx / dist) * speed;
                float moveY = (dy / dist) * speed;
                if (std::abs(moveX) > std::abs(dx)) {
                    move->visualX = targetX;
                } else {
                    move->visualX += moveX;
                }
                if (std::abs(moveY) > std::abs(dy)) {
                    move->visualY = targetY;
                } else {
                    move->visualY += moveY;
                }
            } else {
                move->isMoving = false;
                move->visualX = targetX;
                move->visualY = targetY;
            }

            if (w.get_component<InhabitantTag>(entity)) {
                auto anim = w.get_component<Animation>(entity);
                if (anim) {
                    if (move->isMoving) {
                        anim->currentAnim = "inhabitant_movement_Walking_A";
                        anim->speedMultiplier = (freq / 7.0f);
                    } else {
                        if (anim->currentAnim != "inhabitant_simulation_Push_Ups") {
                            anim->currentAnim = "inhabitant_general_Idle_A";
                        }
                        anim->speedMultiplier = (freq / 50.0f);
                    }
                    if (anim->speedMultiplier < 0.5f) {
                        anim->speedMultiplier = 0.5f;
                    }
                }
            }
        }
    }

    // Process animations
    auto animStorage = w.get_storage<Animation>();
    if (animStorage) {
        for (auto& [entity, anim] : *animStorage) {
            try {
                auto& modelAnim = am.getAnimation(anim->currentAnim);

                anim->currentFrame += GetFrameTime() * anim->baseFps * anim->speedMultiplier;

                if (anim->currentFrame >= modelAnim.keyframeCount) {
                    if (anim->loop) {
                        anim->currentFrame = 0.0f;
                    } else {
                        anim->currentFrame = static_cast<float>(modelAnim.keyframeCount - 1);
                    }
                }
            } catch (const std::exception& e) {
                // Animation not found, ignore
            }
        }
    }
}

} // namespace zappy
