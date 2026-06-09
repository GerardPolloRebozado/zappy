/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerBroadcast.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERBROADCAST_HPP
#define ZAPPY_COMMANDPLAYERBROADCAST_HPP

#include "ACommand.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandPlayerBroadcast : public ACommand {
  public:
    CommandPlayerBroadcast() = default;
    ~CommandPlayerBroadcast() override = default;

    void execute(const std::string& args, World& world) override {
        size_t firstSpace = args.find(' ');
        if (firstSpace == std::string::npos) {
            log_error("Protocol: failed to parse player broadcast args: " + args);
            return;
        }

        std::string idPart = args.substr(0, firstSpace);
        std::string message = args.substr(firstSpace + 1);

        idPart.erase(std::remove(idPart.begin(), idPart.end(), '#'), idPart.end());

        std::istringstream iss(idPart);
        int playerId;
        if (!(iss >> playerId)) {
            log_error("Protocol: failed to parse player broadcast args: " + args);
            return;
        }

        log_info("Protocol: Player #" + std::to_string(playerId) + " broadcasted: " + message);
    }
};
} // namespace zappy
#endif
