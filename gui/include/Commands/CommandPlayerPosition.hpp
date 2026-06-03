/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerPosition.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERPOSITION_HPP
#define ZAPPY_COMMANDPLAYERPOSITION_HPP
#include "ACommand.hpp"

namespace zappy {
    class CommandPlayerPosition : public ACommand {
        CommandPlayerPosition()  = default;
        /**
         * @brief Requests the current position of a specific player.
         * @param args The arguments for the command, expected to contain the player ID
         * @param registry The registry containing the application state, where the player's position will be updated
         */
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDPLAYERPOSITION_HPP
