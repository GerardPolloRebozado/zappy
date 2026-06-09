/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerFork.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERFORK_HPP
#define ZAPPY_COMMANDPLAYERFORK_HPP

#include "ACommand.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandPlayerFork : public ACommand {
  public:
    CommandPlayerFork() = default;
    ~CommandPlayerFork() override = default;

    /**
     * @brief Handles the "pfk" command, indicating a player is laying an egg.
     * @param args The arguments for the command: "#n"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId;

        if (!(iss >> playerId)) {
            log_error("Protocol: failed to parse player fork args: " + args);
            return;
        }
        log_info("Protocol: Player #" + std::to_string(playerId) + " is laying an egg (forking)");
    }
};
} // namespace zappy
#endif
