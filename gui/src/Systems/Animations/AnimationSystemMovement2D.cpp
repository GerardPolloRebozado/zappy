#include "Components/ComponentTags.hpp"
#include "Systems/AnimationSystem.hpp"

namespace zappy {

void AnimationSystem::_update2DMovement(World& w, float freq) {
    auto moveStorage = w.get_storage<MovementInterpolation2D>();
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
                        if (anim->currentAnim != "inhabitant_simulation_Push_Ups" &&
                            anim->currentAnim != "inhabitant_general_Death_A") {
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
}

} // namespace zappy
