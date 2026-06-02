/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerLevel.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERLEVEL_HPP
#define ZAPPY_COMMANDPLAYERLEVEL_HPP
#include "ACommand.hpp"

namespace zappy {
    class CommandPlayerLevel : public ACommand{
        CommandPlayerLevel() = default;
        /**
         * @brief Requests the current level of a specific player.
         * @param args The arguments for the command, expected to contain the player ID.
         * @param registry The registry containing the application state, where the player's level will be updated
         */
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDPLAYERLEVEL_HPP
