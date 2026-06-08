/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** CommandPlayerPosition.hpp
*/

#ifndef ZAPPY_COMMANDPLAYERPOSITION_HPP
#define ZAPPY_COMMANDPLAYERPOSITION_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/ComponentMap.hpp"
#include "ECS/World.hpp"

#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandPlayerPosition : public ACommand {
  public:
    CommandPlayerPosition() = default;
    ~CommandPlayerPosition() override = default;

    /**
     * @brief Handles the "ppo" command, updating a player's position and orientation.
     * @param args The arguments for the command, expected to be "n X Y O" or "#n X Y O"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::ranges::remove(cleanArgs, '#').begin(), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId, x, y, orientation;

        if (!(iss >> playerId >> x >> y >> orientation)) {
            ZAPPY_LOG_E("Protocol: failed to parse player position args: " + args);
            return;
        }
        auto positionsStorage = world.get_storage<Position>();
        if (!positionsStorage) {
            return;
        }
        for (auto& [entity, position] : *positionsStorage) {

            if (entity.id() == playerId) {

                position->x = x;
                position->y = y;

                auto orientationComp = world.get_component<Orientation>(entity);
                if (orientationComp) {
                    orientationComp->current_direction =
                        static_cast<Orientation::Direction>(orientation);
                }
                break;
            }
        }
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDPLAYERPOSITION_HPP