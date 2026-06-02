/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ACommand.hpp
*/
#ifndef ZAPPY_ACOMMAND_HPP
#define ZAPPY_ACOMMAND_HPP
#include "ECS/Register.hpp"
#include <sstream>
#include <iostream>

namespace zappy {
    class ACommand {
    public:
        ~ACommand() = default;
        virtual void execute(const std::string& args, Register& registry) = 0;
    };
} // zappy

#endif //ZAPPY_ACOMMAND_HPP
