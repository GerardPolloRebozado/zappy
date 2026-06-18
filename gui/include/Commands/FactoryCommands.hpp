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
    static ACommand& getCommand(const std::string& commandName);

    static void setChatLogs(std::shared_ptr<ChatLogs> chatLogs);

  private:
    static const std::unordered_map<std::string, std::unique_ptr<ACommand>> _commands;
};
} // namespace zappy

#endif