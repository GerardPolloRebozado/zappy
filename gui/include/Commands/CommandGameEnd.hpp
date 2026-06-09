/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandGameEnd.hpp
*/
#ifndef ZAPPY_COMMANDGAMEEND_HPP
#define ZAPPY_COMMANDGAMEEND_HPP

#include "ACommand.hpp"
#include "Logging/Logger.hpp"
#include <string>

namespace zappy {
class CommandGameEnd : public ACommand {
  public:
    CommandGameEnd() = default;
    ~CommandGameEnd() override = default;

    void execute(const std::string& args, World& world) override {
        log_info("Game ended. Winning team: " + args);
    }
};
} // namespace zappy
#endif
