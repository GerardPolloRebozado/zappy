/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandResourceDrop.hpp
*/
#ifndef ZAPPY_COMMANDRESOURCEDROP_HPP
#define ZAPPY_COMMANDRESOURCEDROP_HPP

#include "AResourceCommand.hpp"
#include "Logging/Logger.hpp"
#include <string>

namespace zappy {
class CommandResourceDrop : public AResourceCommand {
  public:
    CommandResourceDrop() = default;
    ~CommandResourceDrop() override = default;

    void execute(const std::string& args, World& world) override {
        auto [playerId, resourceId, valid] = parseArgs(args);
        if (!valid) {
            return;
        }

        auto [playerEnt, playerPos, found] = findPlayer(playerId, world);
        if (!found) {
            return;
        }

        auto playerInv = world.get_component<Inventory>(playerEnt);
        updateInventory(playerInv.get(), resourceId, -1);
        updateTileInventory(playerPos, resourceId, 1, world);

        log_info("Protocol: Player #" + std::to_string(playerId) + " dropped resource " +
                 std::to_string(resourceId));
    }
};
} // namespace zappy
#endif
