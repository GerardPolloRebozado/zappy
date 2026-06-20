#include "Components/ComponentTags.hpp"
#include "Systems/AnimationSystem.hpp"

namespace zappy {

void AnimationSystem::_update3DMovement(World& w, float freq) {
    auto move3DStorage = w.get_storage<MovementInterpolation3D>();
    if (move3DStorage) {
        std::vector<Entity> entitiesToDespawn;
        for (auto& [entity, move] : *move3DStorage) {
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
            float targetZ = move->targetZ;
            float speed = (freq / 7.0f) * GetFrameTime();

            float dx = targetX - move->visualX;
            float dy = targetY - move->visualY;
            float dz = targetZ - move->visualZ;
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

            if (dist > 0.05f) {
                move->isMoving = true;
                float moveX = (dx / dist) * speed;
                float moveY = (dy / dist) * speed;
                float moveZ = (dz / dist) * speed;
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
                if (std::abs(moveZ) > std::abs(dz)) {
                    move->visualZ = targetZ;
                } else {
                    move->visualZ += moveZ;
                }
            } else {
                move->isMoving = false;
                move->visualX = targetX;
                move->visualY = targetY;
                move->visualZ = targetZ;

                if (w.get_component<AnimatedResource>(entity)) {
                    entitiesToDespawn.push_back(entity);
                }
            }
        }
        for (auto entity : entitiesToDespawn) {
            w.despawn(entity);
        }
    }
}

} // namespace zappy
