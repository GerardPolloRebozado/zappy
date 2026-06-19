/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** FactoryCommands.cpp
*/

#include "Commands/FactoryCommands.hpp"
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
#include "Commands/CommandPlayerExpulsion.hpp"
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
#include "Commands/CommandsErrors.hpp"

namespace zappy {

ACommand& FactoryCommands::getCommand(const std::string& commandName) {
    static const std::unordered_map<std::string, std::unique_ptr<ACommand>> commands = []() {
        std::unordered_map<std::string, std::unique_ptr<ACommand>> map;

        map["msz"] = std::make_unique<CommandMapSize>();
        map["mct"] = std::make_unique<CommandMapContent>();
        map["bct"] = std::make_unique<CommandTileContent>();
        map["tna"] = std::make_unique<CommandTeamNames>();
        map["pnw"] = std::make_unique<CommandPlayerConnection>();
        map["ppo"] = std::make_unique<CommandPlayerPosition>();
        map["plv"] = std::make_unique<CommandPlayerLevel>();
        map["pin"] = std::make_unique<CommandPlayerInventory>();
        map["pbc"] = std::make_unique<CommandPlayerBroadcast>();
        map["pic"] = std::make_unique<CommandIncantationStart>();
        map["pie"] = std::make_unique<CommandIncantationEnd>();
        map["pfk"] = std::make_unique<CommandPlayerFork>();
        map["pex"] = std::make_unique<CommandPlayerExpulsion>();
        map["pdr"] = std::make_unique<CommandResourceDrop>();
        map["pgt"] = std::make_unique<CommandResourceCollect>();
        map["pdi"] = std::make_unique<CommandPlayerDeath>();
        map["enw"] = std::make_unique<CommandEggLayed>();
        map["ebo"] = std::make_unique<CommandEggConnection>();
        map["edi"] = std::make_unique<CommandEggDeath>();
        map["sgt"] = std::make_unique<CommandTimeUpdate>();
        map["sst"] = std::make_unique<CommandTimeUpdate>();
        map["seg"] = std::make_unique<CommandGameEnd>();
        map["smg"] = std::make_unique<CommandServerMessage>();
        map["suc"] = std::make_unique<CommandUnknown>();
        map["sbp"] = std::make_unique<CommandUnknown>();
        return map;
    }();

    auto it = commands.find(commandName);
    if (it != commands.end()) {
        return *it->second;
    }
    throw ErrorProtocol("Unknown command: " + commandName);
}

} // namespace zappy
