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
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDPLAYERLEVEL_HPP
