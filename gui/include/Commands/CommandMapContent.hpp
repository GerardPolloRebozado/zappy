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
        void execute(const std::string&args, Register&registry) override
        {

        };
    };
} // zappy

#endif //ZAPPY_COMMANDMAPCONTENT_HPP
