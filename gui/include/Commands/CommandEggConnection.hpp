/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandEggConnection.hpp
*/
#ifndef ZAPPY_COMMANDEGGCONNECTION_HPP
#define ZAPPY_COMMANDEGGCONNECTION_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandEggConnection : public ACommand {
  public:
    CommandEggConnection() = default;
    ~CommandEggConnection() override = default;

    /**
     * @brief Handles the "ebo" command, player connection for an egg (egg vanishes).
     * @param args The arguments for the command: "#e"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int eggId;

        if (!(iss >> eggId)) {
            ZAPPY_LOG_E("Protocol: failed to parse egg connection args: " + args);
            return;
        }

        auto storage = world.get_storage<Egg>();
        if (storage) {
            for (auto const& [entity, egg] : *storage) {
                if (egg->id == eggId) {
                    world.despawn(entity);
                    ZAPPY_LOG_I("Protocol: Player connected to egg #" + std::to_string(eggId) +
                                " (egg removed)");
                    break;
                }
            }
        }
    }
};
} // namespace zappy
#endif
