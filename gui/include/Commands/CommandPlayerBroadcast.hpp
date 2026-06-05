/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerBroadcast.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERBROADCAST_HPP
#define ZAPPY_COMMANDPLAYERBROADCAST_HPP

#include "ACommand.hpp"
#include <algorithm>
#include <sstream>

namespace zappy {
class CommandPlayerBroadcast : public ACommand {
  public:
    CommandPlayerBroadcast() = default;
    ~CommandPlayerBroadcast() override = default;

    void execute(const std::string& args, World& world) override {
        size_t firstSpace = args.find(' ');
        if (firstSpace == std::string::npos) {
            return;
        }

        std::string idPart = args.substr(0, firstSpace);
        std::string message = args.substr(firstSpace + 1);

        idPart.erase(std::remove(idPart.begin(), idPart.end(), '#'), idPart.end());
        int playerId = std::stoi(idPart);

        std::cout << "Protocol: Player #" << playerId << " broadcasted: " << message << std::endl;
    }
};
} // namespace zappy
#endif
