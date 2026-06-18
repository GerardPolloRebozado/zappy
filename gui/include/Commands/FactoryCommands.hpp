#ifndef FACTORYCOMMANDS_HPP
#define FACTORYCOMMANDS_HPP

#include "Commands/ACommand.hpp"
#include <string>

namespace zappy {
class FactoryCommands {
  public:
    // Throws ErrorProtocol if commandName is unknown.
    static ACommand& getCommand(const std::string& commandName);
};
} // namespace zappy

#endif
