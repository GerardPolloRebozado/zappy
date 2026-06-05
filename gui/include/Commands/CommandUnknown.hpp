/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandUnknown.hpp
*/
#ifndef ZAPPY_COMMANDUNKNOWN_HPP
#define ZAPPY_COMMANDUNKNOWN_HPP

#include "ACommand.hpp"
#include <iostream>

namespace zappy {
class CommandUnknown : public ACommand {
  public:
    CommandUnknown() = default;
    ~CommandUnknown() override = default;

    void execute(const std::string& args, World& world) override {
        std::cout << "Protocol Error: Unknown command or bad parameters received" << std::endl;
    }
};
} // namespace zappy
#endif
