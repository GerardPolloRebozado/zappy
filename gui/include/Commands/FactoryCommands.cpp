/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** FactoryCommands.cpp
*/
#include "FactoryCommands.hpp"
#include "Commands/CommandIncantationEnd.hpp"
#include "Commands/CommandIncantationStart.hpp"
#include "Commands/CommandMapContent.hpp"
#include "Commands/CommandMapSize.hpp"
#include "Commands/CommandPlayerConnection.hpp"
#include "Commands/CommandPlayerInventory.hpp"
#include "Commands/CommandPlayerLevel.hpp"
#include "Commands/CommandPlayerPosition.hpp"
#include "Commands/CommandTeamNames.hpp"
#include "Commands/CommandTileContent.hpp"

namespace zappy {

const std::unordered_map<std::string, FactoryCommands::CommandCreator> FactoryCommands::_creators =
    {{"msz", []() { return std::make_unique<CommandMapSize>(); }},
     {"mct", []() { return std::make_unique<CommandMapContent>(); }},
     {"bct", []() { return std::make_unique<CommandTileContent>(); }},
     {"tna", []() { return std::make_unique<CommandTeamNames>(); }},
     {"ppo", []() { return std::make_unique<CommandPlayerPosition>(); }},
     {"plv", []() { return std::make_unique<CommandPlayerLevel>(); }},
     {"pin", []() { return std::make_unique<CommandPlayerInventory>(); }},
     {"pnw", []() { return std::make_unique<CommandPlayerConnection>(); }},
     {"pic", []() { return std::make_unique<CommandIncantationStart>(); }},
     {"pie", []() { return std::make_unique<CommandIncantationEnd>(); }}};

std::unique_ptr<ACommand> FactoryCommands::createCommand(const std::string& commandName) {
    auto it = _creators.find(commandName);

    if (it != _creators.end()) {
        return it->second();
    }
    return nullptr;
}
} // namespace zappy
