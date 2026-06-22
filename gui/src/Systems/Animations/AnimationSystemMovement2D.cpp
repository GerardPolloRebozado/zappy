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

            const float targetX = static_cast<float>(pos->x);
            const float targetY = static_cast<float>(pos->y);
            const float speed = (freq / 7.0f) * GetFrameTime();

            const float dx = targetX - move->visualX;
            const float dy = targetY - move->visualY;
            const float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 1.5f) {
                // If the distance is more than 1.5 tiles, the player must have teleported
                // (e.g., crossing the map boundaries and wrapping around).
                move->isMoving = false;
                move->visualX = targetX;
                move->visualY = targetY;
            } else if (dist > 0.05f) {
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
