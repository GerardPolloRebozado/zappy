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
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDPLAYERPOSITION_HPP
