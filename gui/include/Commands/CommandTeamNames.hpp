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
        void execute(const std::string&args, Register&registry) override
        {
            std::cout << "Protocol: Team name: " << args << std::endl;
        };
    };
} // zappy

#endif //ZAPPY_COMMANDTEAMNAMES_HPP
