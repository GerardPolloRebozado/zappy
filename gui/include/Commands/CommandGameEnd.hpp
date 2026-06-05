/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandGameEnd.hpp
*/
#ifndef ZAPPY_COMMANDGAMEEND_HPP
#define ZAPPY_COMMANDGAMEEND_HPP

#include "ACommand.hpp"
#include <iostream>

namespace zappy {
class CommandGameEnd : public ACommand {
  public:
    CommandGameEnd() = default;
    ~CommandGameEnd() override = default;

    void execute(const std::string& args, World& world) override {
        std::cout << "Game ended. Winning team: " << args << std::endl;
    }
};
} // namespace zappy
#endif
