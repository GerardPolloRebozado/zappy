/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerInventory.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERINVENTORY_HPP
#define ZAPPY_COMMANDPLAYERINVENTORY_HPP

#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandPlayerInventory : public ACommand {
  public:
    CommandPlayerInventory() = default;
    ~CommandPlayerInventory() override = default;

    /**
     * @brief Handles the "pin" command, updating a player's inventory and position.
     * @param args The arguments for the command: "#n X Y q0 q1 q2 q3 q4 q5 q6"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId, x, y;
        int q0, q1, q2, q3, q4, q5, q6;

        if (!(iss >> playerId >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6)) {
            log_error("Protocol: failed to parse player inventory args: " + args);
            return;
        }

        auto posStorage = world.get_storage<Position>();
        if (!posStorage) {
            return;
        }

        for (auto& [entity, position] : *posStorage) {
            if (entity.id() == (uint32_t)playerId) {
                position->x = x;
                position->y = y;

                auto inv = world.get_component<Inventory>(entity);
                if (inv) {
                    inv->food = q0;
                    inv->linemate = q1;
                    inv->deraumere = q2;
                    inv->sibur = q3;
                    inv->mendiane = q4;
                    inv->phiras = q5;
                    inv->thystame = q6;
                }
                log_info("Protocol: Player #" + std::to_string(playerId) + " inventory updated");
                break;
            }
        }
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDPLAYERINVENTORY_HPP
