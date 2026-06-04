/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** CommandPlayerPosition.hpp
*/

#ifndef ZAPPY_COMMANDPLAYERPOSITION_HPP
#define ZAPPY_COMMANDPLAYERPOSITION_HPP

#include "ACommand.hpp"
#include "ECS/World.hpp"
#include "ECS/ComponentMap.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"

#include <algorithm>
#include <sstream>

#include "Components/ComponentInhabitant.hpp"

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
            cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

            std::istringstream iss(cleanArgs);
            int playerId, x, y, orientation;

            if (!(iss >> playerId >> x >> y >> orientation)) {
                return;
            }
            auto InhabitantStorage = world.get_storage<InhabitantData>();
            if (!InhabitantStorage) {
                return;
            }
            bool found = false;
            Entity targetEntity{0, 0};

            for (auto const& [entity, Inhabitant] : *InhabitantStorage) {
                if (Inhabitant->id == playerId) {
                    targetEntity = entity;
                    found = true;
                    break;
                }
            }

            if (found) {
                auto pos = world.get_component<Position>(targetEntity);
                if (pos) {
                    pos->x = x;
                    pos->y = y;
                } else {
                    world.add_component<Position>(targetEntity, Position{x, y});
                }

                auto Inhabitant = world.get_component<InhabitantData>
                (targetEntity);
                if (Inhabitant) {
                    Inhabitant->orientation = orientation;
                }
            } else {
            }
        }
    };
} // namespace zappy

#endif //ZAPPY_COMMANDPLAYERPOSITION_HPP