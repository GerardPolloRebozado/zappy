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
        int resourceId;
        bool valid;
    };

    CommandArgs parseArgs(const std::string& args) {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());
        std::istringstream iss(cleanArgs);
        int pId, rId;
        if (!(iss >> pId >> rId)) {
            log_error("Protocol: failed to parse resource command args: " + args);
            return {0, 0, false};
        }
        return {pId, rId, true};
    }

    void updateInventory(Inventory* inv, int resourceId, int delta) {
        if (!inv) {
            return;
        }
        switch (resourceId) {
            case 0:
                inv->food += delta;
                inv->exactHp += delta * 126.0f;
                if (inv->exactHp > inv->maxHp) {
                    inv->maxHp = inv->exactHp;
                }
                break;
            case 1:
                inv->linemate += delta;
                break;
            case 2:
                inv->deraumere += delta;
                break;
            case 3:
                inv->sibur += delta;
                break;
            case 4:
                inv->mendiane += delta;
                break;
            case 5:
                inv->phiras += delta;
                break;
            case 6:
                inv->thystame += delta;
                break;
            default:
                break;
        }
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

    void updateTileInventory(Position playerPos, int resourceId, int delta, World& world) {
        auto tileTagStorage = world.get_storage<TileTag>();
        if (!tileTagStorage) {
            return;
        }

        for (auto const& [ent, tag] : *tileTagStorage) {
            auto tPos = world.get_component<Position>(ent);
            if (tPos && tPos->x == playerPos.x && tPos->y == playerPos.y) {
                Entity food = world.spawn();
                world.add_component<AnimatedResource>(food, {resourceId, true});
                world.add_component<Position>(food, tPos);
                world.add_component<Animation>(food, {"", 0.0f, 60.0f, 1.0f, true});
                world.add_component<MovementInterpolation3D>(food, {static_cast<float>(playerPos.x),
                                                                    static_cast<float>(playerPos.y),
                                                                    0.0f, false, 2.0f});
                break;
            }
        }
    }
};
} // namespace zappy

#endif
