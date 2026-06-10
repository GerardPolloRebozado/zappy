/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerLevel.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERLEVEL_HPP
#define ZAPPY_COMMANDPLAYERLEVEL_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandPlayerLevel : public ACommand {
  public:
    CommandPlayerLevel() = default;
    ~CommandPlayerLevel() override = default;

    /**
     * @brief Handles the "plv" command, updating a player's level.
     * @param args The arguments for the command, expected to be "#n L" or "n L"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId, level;

        if (!(iss >> playerId >> level)) {
            log_error("Protocol: failed to parse player level args: " + args);
            return;
        }

        auto levelStorage = world.get_storage<Level>();
        if (!levelStorage) {
            return;
        }

        for (auto& [entity, levelComp] : *levelStorage) {
            auto serverId = world.get_component<ServerId>(entity);
            if (serverId && serverId->id == playerId) {
                levelComp->level = level;
                log_info("Protocol: Player #" + std::to_string(playerId) + " level updated to " +
                         std::to_string(level));
                break;
            }
        }
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDPLAYERLEVEL_HPP
