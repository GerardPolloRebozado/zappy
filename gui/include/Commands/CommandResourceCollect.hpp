/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandResourceCollect.hpp
*/
#ifndef ZAPPY_COMMANDRESOURCECOLLECT_HPP
#define ZAPPY_COMMANDRESOURCECOLLECT_HPP

#include "AResourceCommand.hpp"

namespace zappy {
class CommandResourceCollect : public AResourceCommand {
  public:
    CommandResourceCollect() = default;
    ~CommandResourceCollect() override = default;

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
        updateInventory(playerInv.get(), resourceId, 1);
        updateTileInventory(playerPos, resourceId, -1, world);

        std::cout << "Protocol: Player #" << playerId << " collected resource " << resourceId
                  << std::endl;
    }
};
} // namespace zappy
#endif
