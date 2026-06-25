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

    int mapSizeX = 0, mapSizeY = 0;
    auto mapStorage = w.get_storage<MapTag>();
    if (!mapStorage)
        return;
    for (auto& [mapEntity, mapTagePtr] : *mapStorage) {
        auto mapSize = w.get_component<Size>(mapEntity);
        mapSizeX = mapSize->width;
        mapSizeY = mapSize->height;
    }
    auto celestialBackground = w.get_storage<CelestialObject>();
    if (celestialBackground) {
        for (auto& [entity, celestialPtr] : *celestialBackground) {
            auto& celestial = *celestialPtr;
            float x = (mapSizeX / 2) + (std::max(mapSizeX, mapSizeY) / 1.5f) * cos(celestial.angle);
            float y = (mapSizeY / 2) + (std::max(mapSizeX, mapSizeY) / 1.5f) * sin(celestial.angle);
            celestial.angle += 0.01f;
            celestial.x = x;
            celestial.y = y;
        }
    }
}

} // namespace zappy
