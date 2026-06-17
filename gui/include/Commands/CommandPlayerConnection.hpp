/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerConnection.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERCONNECTION_HPP
#define ZAPPY_COMMANDPLAYERCONNECTION_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandPlayerConnection : public ACommand {
  public:
    CommandPlayerConnection() = default;
    ~CommandPlayerConnection() override = default;

    /**
     * @brief Handles the "pnw" command, spawning a new player.
     * @param args The arguments for the command: "#n X Y O L N"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId, x, y, orientation, level;
        std::string teamName;
        if (!(iss >> playerId >> x >> y >> orientation >> level >> teamName)) {
            log_error("Protocol: failed to parse player connection args: " + args);
            return;
        }

        // Check if player already exists
        Entity player(0, 0);
        bool found = false;
        auto idStorage = world.get_storage<ServerId>();
        if (idStorage) {
            for (auto const& [ent, id] : *idStorage) {
                if (id->id == playerId) {
                    player = ent;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            player = world.spawn();
            world.add_component<ServerId>(player, {playerId});
            world.add_component<InhabitantTag>(player, InhabitantTag{});
        }

        world.add_component<Position>(player, {x, y});
        world.add_component<Orientation>(player,
                                         {static_cast<Orientation::Direction>(orientation)});
        world.add_component<Level>(player, {level});
        world.add_component<TeamName>(player, {teamName, TeamName::findTeam(teamName, world)});
        world.add_component<Inventory>(player, {0, 0, 0, 0, 0, 0, 0});

        log_info("Protocol: Player #" + std::to_string(playerId) +
                 (found ? " updated" : " connected") + " (Team: " + teamName + ")");
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDPLAYERCONNECTION_HPP
