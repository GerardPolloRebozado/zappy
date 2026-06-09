/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandServerMessage.hpp
*/
#ifndef ZAPPY_COMMANDSERVERMESSAGE_HPP
#define ZAPPY_COMMANDSERVERMESSAGE_HPP

#include "ACommand.hpp"
#include "Logging/Logger.hpp"
#include <string>

namespace zappy {
class CommandServerMessage : public ACommand {
  public:
    CommandServerMessage() = default;
    ~CommandServerMessage() override = default;

    void execute(const std::string& args, World& world) override {
        log_info("Server message: " + args);
    }
};
} // namespace zappy
#endif
