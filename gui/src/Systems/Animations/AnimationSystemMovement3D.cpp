#include "Components/ComponentParticleEmitter.hpp"
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

            const float targetX = static_cast<float>(pos->x);
            const float targetY = static_cast<float>(pos->y);
            const float targetZ = move->targetZ;
            float speed = (freq / 7.0f) * GetFrameTime();
            if (w.get_component<AnimatedResource>(entity)) {
                speed = 25.0f * GetFrameTime();
            }

            const float dx = targetX - move->visualX;
            const float dy = targetY - move->visualY;
            const float dz = targetZ - move->visualZ;
            const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

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

                if (auto animRes = w.get_component<AnimatedResource>(entity)) {
                    if (animRes->addToTileOnLand) {
                        auto tileStorage = w.get_storage<TileTag>();
                        if (tileStorage) {
                            for (auto const& [tEnt, tag] : *tileStorage) {
                                auto tPos = w.get_component<Position>(tEnt);
                                if (tPos && tPos->x == pos->x && tPos->y == pos->y) {
                                    if (auto tInv = w.get_component<Inventory>(tEnt)) {
                                        switch (animRes->resourceId) {
                                            case ResourceType::FOOD:
                                                tInv->food++;
                                                break;
                                            case ResourceType::LINEMATE:
                                                tInv->linemate++;
                                                break;
                                            case ResourceType::DERAUMERE:
                                                tInv->deraumere++;
                                                break;
                                            case ResourceType::SIBUR:
                                                tInv->sibur++;
                                                break;
                                            case ResourceType::MENDIANE:
                                                tInv->mendiane++;
                                                break;
                                            case ResourceType::PHIRAS:
                                                tInv->phiras++;
                                                break;
                                            case ResourceType::THYSTAME:
                                                tInv->thystame++;
                                                break;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    entitiesToDespawn.push_back(entity);
                } else if (auto emitter = w.get_component<ComponentParticleEmitter>(entity)) {
                    emitter->isPlaying = false;
                }
            }
        }
        for (auto entity : entitiesToDespawn) {
            w.despawn(entity);
        }
    }
}

} // namespace zappy
