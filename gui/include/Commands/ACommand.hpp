/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ACommand.hpp
*/
#ifndef ZAPPY_ACOMMAND_HPP
#define ZAPPY_ACOMMAND_HPP
#include "ECS/World.hpp"
#include <sstream>

namespace zappy {
class ACommand {
  public:
    virtual ~ACommand() = default;
    virtual void execute(const std::string& args, World& world) = 0;
};
} // namespace zappy

#endif // ZAPPY_ACOMMAND_HPP
