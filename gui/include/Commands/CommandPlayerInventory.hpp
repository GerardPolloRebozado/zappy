/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerInventory.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERINVENTORY_HPP
#define ZAPPY_COMMANDPLAYERINVENTORY_HPP
#include "ACommand.hpp"

namespace zappy {
    class CommandPlayerInventory : public ACommand{
        CommandPlayerInventory() = default;
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDPLAYERINVENTORY_HPP
