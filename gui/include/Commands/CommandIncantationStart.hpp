/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandIncantationStart.hpp
*/
#ifndef ZAPPY_COMMANDINCANTATIONSTART_HPP
#define ZAPPY_COMMANDINCANTATIONSTART_HPP

#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace zappy {
class CommandIncantationStart : public ACommand {
  public:
    CommandIncantationStart() = default;
    ~CommandIncantationStart() override = default;

    /**
     * @brief Handles the "pic" command, indicating the start of an incantation.
     * @param args The arguments for the command: "X Y L #n #n ..."
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int x, y, level;
        std::vector<int> playerIds;
        int playerId;

        if (!(iss >> x >> y >> level)) {
            log_error("Protocol: failed to parse incantation start args: " + args);
            return;
        }

        while (iss >> playerId) {
            playerIds.push_back(playerId);
        }

        log_info("Protocol: Incantation started at (" + std::to_string(x) + ", " +
                    std::to_string(y) + ") for level " + std::to_string(level) + " with " +
                    std::to_string(playerIds.size()) + " players.");
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDINCANTATIONSTART_HPP
