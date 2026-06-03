/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTimeUpdate.hpp
*/
#ifndef ZAPPY_COMMANDTIMEUPDATE_HPP
#define ZAPPY_COMMANDTIMEUPDATE_HPP
#include "ACommand.hpp"

namespace zappy {
    class CommandTimeUpdate : public ACommand{
        CommandTimeUpdate() = default;
        /**
       * @brief Handles the "sgt" command, which provides the current time unit duration in the game.
       * Parses the time unit duration from the command arguments and updates the registry or internal state as needed. This information can be used to adjust the timing of animations, movements, and other time-dependent features in the GUI to ensure they are synchronized with the server's timing.
       * @param args The arguments for the command, expected to contain the time unit duration in milliseconds.
       * @param registry The registry containing the application state, where the time unit duration will be updated
       */
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDTIMEUPDATE_HPP
