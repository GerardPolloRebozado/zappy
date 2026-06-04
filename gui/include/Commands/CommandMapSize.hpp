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
            return;
        }

        Entity mapEntity(0, 0);
        bool found = false;
        auto storage = world.get_storage<Size>();
        if (storage) {
            for (auto const& [ent, size] : *storage) {
                if (size->width == width && size->height == height) {
                    mapEntity = ent;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            mapEntity = world.spawn();
            world.add_component<Size>(mapEntity, {width, height});
            world.add_component<MapTag>(mapEntity, MapTag{});
        }
        std::cout << "Protocol: Map size update [" << args << "]" << std::endl;
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDMAPSIZE_HPP
