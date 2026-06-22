#include "Systems/BackgroundSystem.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentShared.hpp"
#include <cstdlib>
#include "Logging/Logger.hpp"

namespace zappy {

void BackgroundSystem::update(World& w, float dt) {
    auto storage = w.get_storage<BackgroundParallax>();
    if (!storage) {
        return;
    }

    for (auto& [entity, backgroundPtr] : *storage) {
        auto& background = *backgroundPtr;
        background.scrollingBack -= 0.1;
        background.scrollingMid -= 0.5f;
        background.scrollingFore -= 1.0f;

        if (background.scrollingBack <= -background.background.width*2) background.scrollingBack = 0;
        if (background.scrollingMid <= -background.midground.width*2) background.scrollingMid = 0;
        if (background.scrollingFore <= -background.foreground.width*2) background.scrollingFore = 0;
    }
}

} // namespace zappy
