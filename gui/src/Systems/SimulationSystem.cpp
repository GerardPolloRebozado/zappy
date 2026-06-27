/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** SimulationSystem.cpp
*/

#include "Systems/SimulationSystem.hpp"
#include <vector>

namespace zappy {

void SimulationSystem::update(World& world) {
    _handleExpulsions(world);
    _handleResourceCollect(world);
    _handleResourceDrop(world);
}

void SimulationSystem::_handleExpulsions(World& world) {
    auto expStorage = world.get_storage<EventExpulsion>();
    if (!expStorage) {
        return;
    }

    auto sizeIt = world.get_storage<Size>();
    if (!sizeIt || sizeIt->begin() == sizeIt->end()) {
        return;
    }
    int mapWidth = sizeIt->begin()->second->width;
    int mapHeight = sizeIt->begin()->second->height;

    std::vector<Entity> toRemove;
    for (auto const& [executorEntity, exp] : *expStorage) {
        toRemove.push_back(executorEntity);

        auto execPos = world.get_component<Position>(executorEntity);
        auto execOri = world.get_component<Orientation>(executorEntity);
        if (!execPos || !execOri) {
            continue;
        }

        auto posStorage = world.get_storage<Position>();
        if (!posStorage) {
            continue;
        }

        for (auto const& [victimEntity, victimPos] : *posStorage) {
            if (victimEntity == executorEntity) {
                continue;
            }
            if (!world.get_component<InhabitantTag>(victimEntity) ||
                world.get_component<TileTag>(victimEntity)) {
                continue;
            }

            if (victimPos->x == execPos->x && victimPos->y == execPos->y) {
                _moveForward(victimPos, execOri->current_direction, mapWidth, mapHeight);
                world.add_component<EventExpulsed>(victimEntity, EventExpulsed{});
            }
        }
    }
}

void SimulationSystem::_handleResourceCollect(World& world) {
    auto evtStorage = world.get_storage<EventResourceCollect>();
    if (!evtStorage) {
        return;
    }

    std::vector<Entity> toRemove;
    for (auto const& [entity, evt] : *evtStorage) {
        toRemove.push_back(entity);

        auto inv = world.get_component<Inventory>(entity);
        if (inv) {
            _updateInventory(inv.get(), evt->resourceId, 1);
        }

        auto pos = world.get_component<Position>(entity);
        if (!pos) {
            continue;
        }

        auto tileStorage = world.get_storage<TileTag>();
        if (!tileStorage) {
            continue;
        }

        for (auto const& [tileEnt, tag] : *tileStorage) {
            auto tPos = world.get_component<Position>(tileEnt);
            if (tPos && tPos->x == pos->x && tPos->y == pos->y) {
                auto tileInv = world.get_component<Inventory>(tileEnt);
                if (tileInv) {
                    _updateInventory(tileInv.get(), evt->resourceId, -1);
                }

                // Visual particle entity
                Entity food = world.spawn();
                world.add_component<AnimatedResource>(food, {evt->resourceId, false});
                world.add_component<Position>(food, tPos);
                world.add_component<Animation>(food, {"", 0.0f, 60.0f, 1.0f, true});
                world.add_component<MovementInterpolation3D>(
                    food,
                    {static_cast<float>(pos->x), static_cast<float>(pos->y), 0.0f, false, 2.0f});
                break;
            }
        }
    }

    for (auto e : toRemove) {
        world.remove_component<EventResourceCollect>(e);
    }
}

void SimulationSystem::_handleResourceDrop(World& world) {
    auto evtStorage = world.get_storage<EventResourceDrop>();
    if (!evtStorage) {
        return;
    }

    std::vector<Entity> toRemove;
    for (auto const& [entity, evt] : *evtStorage) {
        toRemove.push_back(entity);

        auto inv = world.get_component<Inventory>(entity);
        if (inv) {
            _updateInventory(inv.get(), evt->resourceId, -1);
        }

        auto pos = world.get_component<Position>(entity);
        if (!pos) {
            continue;
        }

        auto tileStorage = world.get_storage<TileTag>();
        if (!tileStorage) {
            continue;
        }

        for (auto const& [tileEnt, tag] : *tileStorage) {
            auto tPos = world.get_component<Position>(tileEnt);
            if (tPos && tPos->x == pos->x && tPos->y == pos->y) {
                auto tileInv = world.get_component<Inventory>(tileEnt);
                if (tileInv) {
                    _updateInventory(tileInv.get(), evt->resourceId, 1);
                }

                // Visual particle entity
                Entity food = world.spawn();
                world.add_component<AnimatedResource>(food, {evt->resourceId, true});
                world.add_component<Position>(food, tPos);
                world.add_component<Animation>(food, {"", 0.0f, 60.0f, 1.0f, true});
                world.add_component<MovementInterpolation3D>(
                    food,
                    {static_cast<float>(pos->x), static_cast<float>(pos->y), 0.0f, false, 2.0f});
                break;
            }
        }
    }

    for (auto e : toRemove) {
        world.remove_component<EventResourceDrop>(e);
    }
}

void SimulationSystem::_moveForward(const std::shared_ptr<Position>& pos,
                                    Orientation::Direction direction, int mapWidth, int mapHeight) {
    switch (direction) {
        case Orientation::N:
            pos->y = (pos->y + mapHeight - 1) % mapHeight;
            break;
        case Orientation::E:
            pos->x = (pos->x + 1) % mapWidth;
            break;
        case Orientation::S:
            pos->y = (pos->y + 1) % mapHeight;
            break;
        case Orientation::W:
            pos->x = (pos->x + mapWidth - 1) % mapWidth;
            break;
    }
}

void SimulationSystem::_updateInventory(Inventory* inv, ResourceType resourceId, int delta) {
    if (!inv) {
        return;
    }
    switch (resourceId) {
        case ResourceType::FOOD:
            inv->food += delta;
            inv->exactHp += delta * 126.0f;
            if (inv->exactHp > inv->maxHp) {
                inv->maxHp = inv->exactHp;
            }
            break;
        case ResourceType::LINEMATE:
            inv->linemate += delta;
            break;
        case ResourceType::DERAUMERE:
            inv->deraumere += delta;
            break;
        case ResourceType::SIBUR:
            inv->sibur += delta;
            break;
        case ResourceType::MENDIANE:
            inv->mendiane += delta;
            break;
        case ResourceType::PHIRAS:
            inv->phiras += delta;
            break;
        case ResourceType::THYSTAME:
            inv->thystame += delta;
            break;
        default:
            break;
    }
}

} // namespace zappy
