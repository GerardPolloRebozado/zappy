/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTeamNames.hpp
*/
#ifndef ZAPPY_COMMANDTEAMNAMES_HPP
#define ZAPPY_COMMANDTEAMNAMES_HPP

#include "ACommand.hpp"
#include "Color.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentTags.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <ostream>
#include <string>

const std::vector<raylib::Color> TEAM_COLORS = {
    raylib::Color(230, 60, 60, 255),  // Red
    raylib::Color(60, 230, 60, 255),  // Green
    raylib::Color(60, 100, 230, 255), // Blue
    raylib::Color(230, 230, 60, 255), // Yellow
    raylib::Color(230, 60, 230, 255), // Magenta
    raylib::Color(60, 230, 230, 255)  // Cyan
};

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
                if (name && name->_team_name == teamName) {
                    return; // Team already exists
                }
            }
        }

        Entity teamEntity = world.spawn();
        auto team_storage = world.get_storage<TeamTag>();
        int n_teams = 0;
        if (team_storage != nullptr) {
            n_teams = team_storage->size();
        }
        TeamName team(teamName, TEAM_COLORS[n_teams % TEAM_COLORS.size()]);
        world.add_component<TeamName>(teamEntity, team);
        world.add_component<TeamTag>(teamEntity, TeamTag{});
        log_info("Protocol: Team added: " + teamName);
    };
};
} // namespace zappy

#endif // ZAPPY_COMMANDTEAMNAMES_HPP
