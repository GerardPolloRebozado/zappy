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
    raylib::Color(180, 80, 80, 255),  raylib::Color(80, 160, 100, 255),
    raylib::Color(80, 110, 180, 255), raylib::Color(200, 160, 70, 255),
    raylib::Color(160, 90, 160, 255), raylib::Color(70, 160, 180, 255)};

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

        auto teamStorage = world.get_storage<TeamTag>();
        auto nameStorage = world.get_storage<TeamName>();
        bool teamFound = false;
        if (teamStorage && nameStorage) {
            for (auto const& [ent, tag] : *teamStorage) {
                auto name = world.get_component<TeamName>(ent);
                if (name && name->_team_name == teamName) {
                    teamFound = true;
                    break;
                }
            }
        }
        if (teamFound) {
            return;
        }

        Entity teamEntity = world.spawn();
        int n_teams = teamStorage ? teamStorage->size() : 0;
        raylib::Color color = TEAM_COLORS[n_teams % TEAM_COLORS.size()];
        TeamName team(teamName, color);
        world.add_component<TeamName>(teamEntity, team);
        world.add_component<TeamTag>(teamEntity, TeamTag{});

        // apply the color to any players that were spawned before this team was registered
        if (nameStorage) {
            for (auto& [ent, name] : *nameStorage) {
                if (name && name->_team_name == teamName) {
                    name->_color = color;
                }
            }
        }
        log_info("Protocol: Team added: " + teamName);
    };
};
} // namespace zappy

#endif // ZAPPY_COMMANDTEAMNAMES_HPP
