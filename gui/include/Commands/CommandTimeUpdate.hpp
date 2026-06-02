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
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDTIMEUPDATE_HPP
