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
#include <algorithm>
#include <sstream>

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
            return;
        }

        Entity egg = world.spawn();
        world.add_component<Position>(egg, {x, y});
        world.add_component<Egg>(egg, {eggId});
        world.add_component<EggTag>(egg, EggTag{});

        std::cout << "Protocol: Egg #" << eggId << " layed by player #" << playerId << " at (" << x
                  << ", " << y << ")" << std::endl;
    }
};
} // namespace zappy
#endif
