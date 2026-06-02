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
        /**
         * @brief Requests the inventory content of a specific player.
         * @param args The arguments for the command, expected to contain the player ID.
         * @param registry The registry containing the application state, where the player's inventory will be updated
         */
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDPLAYERINVENTORY_HPP
