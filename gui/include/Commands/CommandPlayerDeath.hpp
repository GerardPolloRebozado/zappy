/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerDeath.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERDEATH_HPP
#define ZAPPY_COMMANDPLAYERDEATH_HPP

#include "ACommand.hpp"
#include <algorithm>
#include <sstream>

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
            return;
        }

        auto posStorage = world.get_storage<Position>();
        if (posStorage) {
            for (auto const& [entity, pos] : *posStorage) {
                if (entity.id() == (uint32_t)playerId && !world.get_component<TileTag>(entity)) {
                    world.despawn(entity);
                    std::cout << "Protocol: Player #" << playerId << " died" << std::endl;
                    break;
                }
            }
        }
    }
};
} // namespace zappy
#endif
