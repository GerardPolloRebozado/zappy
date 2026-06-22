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

void ParticleSystem::_handleJumps(World& w) {
    printf("Into handle_jump\n");
    auto eventJumpStorage = w.get_storage<EventJump>();
    if (eventJumpStorage) {
        printf("into handle jump storage\n");
        std::vector<Entity> eventJumpToRemove;
        for (auto const& [entity, event] : *eventJumpStorage) {
            auto posComponent = w.get_component<Position>(entity);
            if (posComponent) {
                printf("into hand jump storage posComponnet\n");
                auto anim = w.get_component<Animation>(entity);
                if (anim) {
                    anim->currentAnim = "inhabitant_movement_Jump_Full_Short";
                    anim->loop = false;
                    anim->currentFrame = 0.0f;
                }

                auto move = w.get_component<MovementInterpolation2D>(entity);
                if (move) {
                    move->isMoving = false;
                }
            }
            eventJumpToRemove.push_back(entity);
        }
        for (const auto& e : eventJumpToRemove) {
            w.remove_component<EventJump>(e);
        }
    }
}

} // namespace zappy
