#include "Systems/BackgroundSystem.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include <cstdlib>
#include "Logging/Logger.hpp"

namespace zappy {

// Update the position of the different layers of background and the celestials objects
void BackgroundSystem::update(World& w, float dt) {
    auto backgroundStorage = w.get_storage<BackgroundParallax>();
    if (backgroundStorage) {
        for (auto& [entity, backgroundPtr] : *backgroundStorage) {
            auto& background = *backgroundPtr;
            background.scrollingBack -= 0.1;
            background.scrollingMid -= 0.5f;
            background.scrollingFore -= 1.0f;

            if (background.scrollingBack <= -background.background.width*2) background.scrollingBack = 0;
            if (background.scrollingMid <= -background.midground.width*2) background.scrollingMid = 0;
            if (background.scrollingFore <= -background.foreground.width*2) background.scrollingFore = 0;
        }
    }

    auto celestialBackground = w.get_storage<CelestialObject>();
    if (celestialBackground) {
        for (auto& [entity, celestialPtr] : *celestialBackground) {
            auto& celestial = *celestialPtr;
            // auto posComponent = w.get_component<Position>(entity);
            float x = 5.0f + 5.0f * cos(celestial.angle);
            float y = 5.0f + 5.0f * sin(celestial.angle);
            celestial.angle += 0.01f;
            celestial.x = x;
            celestial.y = y;
            // posComponent->x = x;
            // posComponent->y = y;
        }
    }
}

} // namespace zappy
