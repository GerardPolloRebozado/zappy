/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandResourceDrop.hpp
*/
#ifndef ZAPPY_COMMANDRESOURCEDROP_HPP
#define ZAPPY_COMMANDRESOURCEDROP_HPP

#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include <algorithm>
#include <sstream>

namespace zappy {
class CommandResourceDrop : public ACommand {
  public:
    CommandResourceDrop() = default;
    ~CommandResourceDrop() override = default;

    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId, resourceId;

        if (!(iss >> playerId >> resourceId)) {
            return;
        }

        auto invStorage = world.get_storage<Inventory>();
        auto posStorage = world.get_storage<Position>();
        if (!invStorage || !posStorage) {
            return;
        }

        Entity playerEnt(0, 0);
        Position playerPos = {0, 0};
        bool found = false;

        for (auto const& [ent, pos] : *posStorage) {
            if (ent.id() == (uint32_t)playerId && !world.get_component<TileTag>(ent)) {
                playerEnt = ent;
                playerPos = *pos;
                found = true;
                break;
            }
        }

        if (!found) {
            return;
        }

        auto playerInv = invStorage->get(playerEnt);
        if (playerInv) {
            _updateInventory(playerInv.get(), resourceId, -1);
        }

        auto tileTagStorage = world.get_storage<TileTag>();
        if (tileTagStorage) {
            for (auto const& [ent, tag] : *tileTagStorage) {
                auto tPos = world.get_component<Position>(ent);
                if (tPos && tPos->x == playerPos.x && tPos->y == playerPos.y) {
                    auto tileInv = invStorage->get(ent);
                    if (tileInv) {
                        _updateInventory(tileInv.get(), resourceId, 1);
                    }
                    break;
                }
            }
        }
        std::cout << "Protocol: Player #" << playerId << " dropped resource " << resourceId
                  << std::endl;
    }

  private:
    void _updateInventory(Inventory* inv, int resourceId, int delta) {
        switch (resourceId) {
            case 0:
                inv->food += delta;
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
};
} // namespace zappy
#endif
