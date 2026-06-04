/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTeamNames.hpp
*/
#ifndef ZAPPY_COMMANDTEAMNAMES_HPP
#define ZAPPY_COMMANDTEAMNAMES_HPP
#include "ACommand.hpp"

namespace zappy {
class CommandTeamNames : public ACommand {
  public:
    CommandTeamNames() = default;
    /**
     * @brief Handles the "tna" command, which provides the list of team names in the game.
     * @param args The arguments for the command.
     * @param world The world containing the application state, where the team names will be updated
     */
    void execute(const std::string& args, World& world) override {
        std::cout << "Protocol: Team name: " << args << std::endl;
    };
};
} // namespace zappy

#endif // ZAPPY_COMMANDTEAMNAMES_HPP
