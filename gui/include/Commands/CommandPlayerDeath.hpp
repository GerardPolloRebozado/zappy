/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerDeath.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERDEATH_HPP
#define ZAPPY_COMMANDPLAYERDEATH_HPP

#include "ACommand.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandPlayerDeath : public ACommand {
  public:
    CommandPlayerDeath() = default;
    ~CommandPlayerDeath() override = default;

    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId;

        if (!(iss >> playerId)) {
            log_error("Protocol: failed to parse player death args: " + args);
            return;
        }

        auto posStorage = world.get_storage<Position>();
        if (posStorage) {
            for (auto const& [entity, pos] : *posStorage) {
                auto serverId = world.get_component<ServerId>(entity);
                if (serverId && serverId->id == playerId && !world.get_component<TileTag>(entity)) {
                    world.add_component<EventDeath>(entity, EventDeath{});
                    Entity tombEntity = world.spawn();
                    world.add_component<TombTag>(tombEntity, TombTag{});
                    world.add_component<Position>(tombEntity, pos);
                    log_info("Protocol: Player #" + std::to_string(playerId) + " died");
                    break;
                }
            }
        }
    }
};
} // namespace zappy
#endif
