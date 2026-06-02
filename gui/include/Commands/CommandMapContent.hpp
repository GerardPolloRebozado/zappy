/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandMapContent.hpp
*/
#ifndef ZAPPY_COMMANDMAPCONTENT_HPP
#define ZAPPY_COMMANDMAPCONTENT_HPP
#include "ACommand.hpp"
namespace zappy {
    class CommandMapContent : public ACommand {
    public:
        CommandMapContent() = default;
        ~CommandMapContent() = default;
        /**
         * @brief Sends a request for the content of all tiles on the map.
         * @param args The arguments for the command.
         * @param registry The registry containing the application state.
         */
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDMAPCONTENT_HPP
