/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandUnknown.hpp
*/
#ifndef ZAPPY_COMMANDUNKNOWN_HPP
#define ZAPPY_COMMANDUNKNOWN_HPP

#include "ACommand.hpp"
#include "Logging/Logger.hpp"
#include <string>

namespace zappy {
class CommandUnknown : public ACommand {
  public:
    CommandUnknown() = default;
    ~CommandUnknown() override = default;

    void execute(const std::string& args, World& world) override {
        log_error("Protocol Error: Unknown command or bad parameters received");
    }
};
} // namespace zappy
#endif
