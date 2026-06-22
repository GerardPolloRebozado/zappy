/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerBroadcast.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERBROADCAST_HPP
#define ZAPPY_COMMANDPLAYERBROADCAST_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
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

        std::string teamStr = "";
        auto posStorage = world.get_storage<Position>();
        if (posStorage) {
            for (auto const& [entity, pos] : *posStorage) {
                auto serverId = world.get_component<ServerId>(entity);
                if (serverId && serverId->id == playerId) {
                    world.add_component<EventBroadcast>(entity, EventBroadcast{});
                    auto teamComp = world.get_component<TeamName>(entity);
                    if (teamComp) {
                        teamStr = teamComp->_team_name;
                    }
                    break;
                }
            }
        }

        if (_chatLogs) {
            _chatLogs->addChatLog("Player #" + std::to_string(playerId) + " shouts: " + message,
                                  "BROADCAST", teamStr);
        }
        log_info("Protocol: Player #" + std::to_string(playerId) + " broadcasted: " + message);
    }
};
} // namespace zappy
#endif