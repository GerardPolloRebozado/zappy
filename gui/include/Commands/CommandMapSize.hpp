/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandMapSize.hpp
*/
#ifndef ZAPPY_COMMANDMAPSIZE_HPP
#define ZAPPY_COMMANDMAPSIZE_HPP
#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Logging/Logger.hpp"
#include <string>

namespace zappy {
class CommandMapSize : public ACommand {
  public:
    CommandMapSize() = default;
    /**
     * @brief Handles the "msz" command, which provides the dimensions of the map.
     * Parses the width and height from the command arguments and updates the world.
     * If an entity for the map size doesn't exist, it creates one and assigns the Size component.
     * @param args The arguments for the command, expected to be in the format "width height".
     * @param world The world containing the application state, where the map size will
     */
    void execute(const std::string& args, World& world) override {
        std::istringstream iss(args);
        int width, height;
        if (!(iss >> width >> height)) {
            log_error("Protocol: failed to parse map size args: " + args);
            return;
        }

        static std::optional<Entity> mapEntityCache;
        Entity mapEntity(0, 0);
        bool found = false;

        if (mapEntityCache && world.is_alive(*mapEntityCache)) {
            mapEntity = *mapEntityCache;
            found = true;
        } else {
            auto storage = world.get_storage<Size>();
            if (storage) {
                for (auto const& [ent, size] : *storage) {
                    if (size->width == width && size->height == height) {
                        mapEntity = ent;
                        found = true;
                        mapEntityCache = ent;
                        break;
                    }
                }
            }
        }

        if (!found) {
            mapEntity = world.spawn();
            world.add_component<Size>(mapEntity, {width, height});
            world.add_component<MapTag>(mapEntity, MapTag{});
            mapEntityCache = mapEntity;
        }
        log_info("Protocol: Map size update [" + args + "]");
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDMAPSIZE_HPP
