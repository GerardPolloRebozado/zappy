/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** FactoryCommands.cpp
*/
#include "FactoryCommands.hpp"
#include "Commands/CommandEggConnection.hpp"
#include "Commands/CommandEggDeath.hpp"
#include "Commands/CommandEggLayed.hpp"
#include "Commands/CommandGameEnd.hpp"
#include "Commands/CommandIncantationEnd.hpp"
#include "Commands/CommandIncantationStart.hpp"
#include "Commands/CommandMapContent.hpp"
#include "Commands/CommandMapSize.hpp"
#include "Commands/CommandPlayerBroadcast.hpp"
#include "Commands/CommandPlayerConnection.hpp"
#include "Commands/CommandPlayerDeath.hpp"
#include "Commands/CommandPlayerFork.hpp"
#include "Commands/CommandPlayerInventory.hpp"
#include "Commands/CommandPlayerLevel.hpp"
#include "Commands/CommandPlayerPosition.hpp"
#include "Commands/CommandResourceCollect.hpp"
#include "Commands/CommandResourceDrop.hpp"
#include "Commands/CommandServerMessage.hpp"
#include "Commands/CommandTeamNames.hpp"
#include "Commands/CommandTileContent.hpp"
#include "Commands/CommandTimeUpdate.hpp"
#include "Commands/CommandUnknown.hpp"

namespace zappy {

const std::unordered_map<std::string, FactoryCommands::CommandCreator> FactoryCommands::_creators =
    {{"msz", []() { return std::make_unique<CommandMapSize>(); }},
     {"mct", []() { return std::make_unique<CommandMapContent>(); }},
     {"bct", []() { return std::make_unique<CommandTileContent>(); }},
     {"tna", []() { return std::make_unique<CommandTeamNames>(); }},
     {"pnw", []() { return std::make_unique<CommandPlayerConnection>(); }},
     {"ppo", []() { return std::make_unique<CommandPlayerPosition>(); }},
     {"plv", []() { return std::make_unique<CommandPlayerLevel>(); }},
     {"pin", []() { return std::make_unique<CommandPlayerInventory>(); }},
     {"pbc", []() { return std::make_unique<CommandPlayerBroadcast>(); }},
     {"pic", []() { return std::make_unique<CommandIncantationStart>(); }},
     {"pie", []() { return std::make_unique<CommandIncantationEnd>(); }},
     {"pfk", []() { return std::make_unique<CommandPlayerFork>(); }},
     {"pdr", []() { return std::make_unique<CommandResourceDrop>(); }},
     {"pgt", []() { return std::make_unique<CommandResourceCollect>(); }},
     {"pdi", []() { return std::make_unique<CommandPlayerDeath>(); }},
     {"enw", []() { return std::make_unique<CommandEggLayed>(); }},
     {"ebo", []() { return std::make_unique<CommandEggConnection>(); }},
     {"edi", []() { return std::make_unique<CommandEggDeath>(); }},
     {"sgt", []() { return std::make_unique<CommandTimeUpdate>(); }},
     {"sst", []() { return std::make_unique<CommandTimeUpdate>(); }},
     {"seg", []() { return std::make_unique<CommandGameEnd>(); }},
     {"smg", []() { return std::make_unique<CommandServerMessage>(); }},
     {"suc", []() { return std::make_unique<CommandUnknown>(); }},
     {"sbp", []() { return std::make_unique<CommandUnknown>(); }}};

std::unique_ptr<ACommand> FactoryCommands::createCommand(const std::string& commandName) {
    auto it = _creators.find(commandName);

    if (it != _creators.end()) {
        return it->second();
    }
    return nullptr;
}
} // namespace zappy
