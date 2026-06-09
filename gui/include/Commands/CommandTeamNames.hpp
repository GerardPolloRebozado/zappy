/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTeamNames.hpp
*/
#ifndef ZAPPY_COMMANDTEAMNAMES_HPP
#define ZAPPY_COMMANDTEAMNAMES_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentTags.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <string>

namespace zappy {
class CommandTeamNames : public ACommand {
  public:
    CommandTeamNames() = default;
    ~CommandTeamNames() override = default;

    /**
     * @brief Handles the "tna" command, which provides the list of team names in the game.
     * @param args The arguments for the command, expected to be the team name.
     * @param world The world containing the application state, where the team names will be updated
     */
    void execute(const std::string& args, World& world) override {
        std::string teamName = args;

        // Trim leading and trailing whitespace
        teamName.erase(0, teamName.find_first_not_of(" \t\n\r"));
        teamName.erase(teamName.find_last_not_of(" \t\n\r") + 1);

        if (teamName.empty()) {
            log_error("Protocol: failed to parse team name args: " + args);
            return;
        }

        auto storage = world.get_storage<TeamName>();
        if (storage) {
            for (auto const& [ent, name] : *storage) {
                if (name->team_name == teamName) {
                    return; // Team already exists
                }
            }
        }

        Entity teamEntity = world.spawn();
        world.add_component<TeamName>(teamEntity, {teamName});
        world.add_component<TeamTag>(teamEntity, TeamTag{});

        log_info("Protocol: Team added: " + teamName);
    };
};
} // namespace zappy

#endif // ZAPPY_COMMANDTEAMNAMES_HPP
