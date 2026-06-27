/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** AResourceCommand.hpp
*/

#ifndef ZAPPY_ARESOURCECOMMAND_HPP
#define ZAPPY_ARESOURCECOMMAND_HPP

#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/Entity.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class AResourceCommand : public ACommand {
  protected:
    struct CommandArgs {
        int playerId;
        ResourceType resourceId;
        bool valid;
    };

    CommandArgs parseArgs(const std::string& args) {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());
        std::istringstream iss(cleanArgs);
        int pId, rId;
        if (!(iss >> pId >> rId)) {
            log_error("Protocol: failed to parse resource command args: " + args);
            return {0, static_cast<ResourceType>(0), false};
        }
        return {pId, static_cast<ResourceType>(rId), true};
    }

    struct PlayerContext {
        Entity entity;
        Position pos;
        bool found;
    };

    PlayerContext findPlayer(int playerId, World& world) {
        auto posStorage = world.get_storage<Position>();
        if (!posStorage) {
            return {Entity(0, 0), {0, 0}, false};
        }

        for (auto const& [ent, pos] : *posStorage) {
            auto serverId = world.get_component<ServerId>(ent);
            if (serverId && serverId->id == playerId && !world.get_component<TileTag>(ent)) {
                return {ent, *pos, true};
            }
        }
        return {Entity(0, 0), {0, 0}, false};
    }
};
} // namespace zappy

#endif
