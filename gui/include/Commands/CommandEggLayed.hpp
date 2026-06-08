/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandEggLayed.hpp
*/
#ifndef ZAPPY_COMMANDEGGLAYED_HPP
#define ZAPPY_COMMANDEGGLAYED_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandEggLayed : public ACommand {
  public:
    CommandEggLayed() = default;
    ~CommandEggLayed() override = default;

    /**
     * @brief Handles the "enw" command, indicating an egg was laid.
     * @param args The arguments for the command: "#e #n X Y"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int eggId, playerId, x, y;

        if (!(iss >> eggId >> playerId >> x >> y)) {
            ZAPPY_LOG_E("Protocol: failed to parse egg layed args: " + args);
            return;
        }

        Entity egg = world.spawn();
        world.add_component<Position>(egg, {x, y});
        world.add_component<Egg>(egg, {eggId});
        world.add_component<EggTag>(egg, EggTag{});

        ZAPPY_LOG_I("Protocol: Egg #" + std::to_string(eggId) + " layed by player #" +
                    std::to_string(playerId) + " at (" + std::to_string(x) + ", " +
                    std::to_string(y) + ")");
    }
};
} // namespace zappy
#endif
