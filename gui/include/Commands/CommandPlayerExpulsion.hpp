/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerExpulsion.hpp
*/

#ifndef ZAPPY_COMMANDPLAYEREXPULSION_HPP
#define ZAPPY_COMMANDPLAYEREXPULSION_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <sstream>

namespace zappy {
class CommandPlayerExpulsion : public ACommand {
  public:
    CommandPlayerExpulsion() = default;
    ~CommandPlayerExpulsion() override = default;

    /**
     * @brief Handles the "pex" command, moving an ejected player one tile forward
     *        in the ejector's facing direction (inferred from a co-tile inhabitant).
     * @param args The arguments for the command: "#n"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId;

        if (!(iss >> playerId)) {
            return;
        }

        auto posStorage = world.get_storage<Position>();
        if (!posStorage) {
            return;
        }

        auto victim = findVictim(world, posStorage, playerId);
        if (!victim) {
            return;
        }

        auto ejectDirection = findEjectDirection(world, posStorage, victim->position, playerId);
        if (!ejectDirection) {
            return;
        }

        auto sizeStorage = world.get_storage<Size>();
        if (!sizeStorage) {
            return;
        }
        auto sizeIt = sizeStorage->begin();
        if (sizeIt == sizeStorage->end()) {
            return;
        }

        int mapWidth = sizeIt->second->width;
        int mapHeight = sizeIt->second->height;

        move_forward(victim->position, *ejectDirection, mapWidth, mapHeight);
        std::cout << "Protocol: Player #" << playerId << " expelled" << std::endl;
    }

  private:
    struct Victim {
        Entity entity;
        std::shared_ptr<Position> position;
    };

    static std::optional<Victim> findVictim(World& world, const std::shared_ptr<ComponentMap<Position>>& posStorage,
                                            int playerId) {
        for (auto const& [entity, pos] : *posStorage) {
            if (entity.id() != static_cast<uint32_t>(playerId)) {
                continue;
            }
            if (!world.get_component<InhabitantTag>(entity) || world.get_component<TileTag>(entity)) {
                continue;
            }
            auto position = world.get_component<Position>(entity);
            if (!position) {
                return std::nullopt;
            }
            return Victim{entity, position};
        }
        return std::nullopt;
    }

    static std::optional<Orientation::Direction>
    findEjectDirection(World& world, const std::shared_ptr<ComponentMap<Position>>& posStorage,
                       const std::shared_ptr<Position>& victimPos, int victimId) {
        for (auto const& [entity, pos] : *posStorage) {
            if (entity.id() == static_cast<uint32_t>(victimId)) {
                continue;
            }
            if (!world.get_component<InhabitantTag>(entity) || world.get_component<TileTag>(entity)) {
                continue;
            }
            if (pos->x != victimPos->x || pos->y != victimPos->y) {
                continue;
            }
            auto orientation = world.get_component<Orientation>(entity);
            if (!orientation) {
                continue;
            }
            return orientation->current_direction;
        }
        return std::nullopt;
    }

    static void move_forward(const std::shared_ptr<Position>& pos, Orientation::Direction direction, int mapWidth,
                             int mapHeight) {
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
};
} // namespace zappy
#endif
