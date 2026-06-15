#ifndef FACTORYCOMMANDS_HPP
#define FACTORYCOMMANDS_HPP

#include "ACommand.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace zappy {
class FactoryCommands {
  public:
    // Throws ErrorProtocol if commandName is unknown.
    static std::unique_ptr<ACommand> createCommand(const std::string& commandName);

  private:
    using CommandCreator = std::function<std::unique_ptr<ACommand>()>;
    static const std::unordered_map<std::string, CommandCreator> _creators;
};
} // namespace zappy

#endif