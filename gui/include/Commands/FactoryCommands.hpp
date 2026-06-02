#ifndef FACTORYCOMMANDS_HPP
#define FACTORYCOMMANDS_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include "ACommand.hpp"

namespace zappy {
    class FactoryCommands {
    public:
        static std::unique_ptr<ACommand> createCommand(const std::string& commandName);

    private:
        using CommandCreator = std::function<std::unique_ptr<ACommand>()>;
        static const std::unordered_map<std::string, CommandCreator> _creators;
    };
}

#endif