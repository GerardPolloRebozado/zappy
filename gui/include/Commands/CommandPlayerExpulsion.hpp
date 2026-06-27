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
#include "Logging/LogConfig.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

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
        int executorId = parseArgs(args);
        if (executorId < 0) {
            return;
        }

        auto posStorage = world.get_storage<Position>();
        if (!posStorage) {
            return;
        }

        auto executorData = findExecutor(world, posStorage, executorId);
        if (!executorData) {
            return;
        }

        auto [executorEntity, executorPos, executorOri] = *executorData;

        // Add Expulsions effects
        world.add_component<EventExpulsion>(executorEntity, EventExpulsion{});

        auto victims = findVictims(world, posStorage, executorEntity, executorPos);
        if (victims.empty()) {
            log_info("No players to eject in this tile");
            return;
        }

        auto sizeIt = world.get_storage<Size>();
        if (!sizeIt || sizeIt->begin() == sizeIt->end()) {
            return;
        }
        int mapWidth = sizeIt->begin()->second->width;
        int mapHeight = sizeIt->begin()->second->height;

        for (auto& victimPos : victims) {
            move_forward(victimPos, executorOri->current_direction, mapWidth, mapHeight);
        }
        log_info("Protocol: Player #" + std::to_string(executorId) +
                 " expelled everyone on their tile");
    }

  private:
    struct ExecutorData {
        Entity entity;
        std::shared_ptr<Position> pos;
        std::shared_ptr<Orientation> ori;
    };

    static int parseArgs(const std::string& args) {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId;
        if (!(iss >> playerId)) {
            log_error("Protocol: failed to parse player expulsion args: " + args);
            return -1;
        }
        return playerId;
    }

    static std::optional<ExecutorData>
    findExecutor(World& world, const std::shared_ptr<ComponentMap<Position>>& posStorage,
                 int executorId) {
        for (auto const& [entity, pos] : *posStorage) {
            auto serverId = world.get_component<ServerId>(entity);
            if (serverId && serverId->id == executorId) {
                auto ori = world.get_component<Orientation>(entity);
                if (pos && ori) {
                    return ExecutorData{entity, pos, ori};
                }
            }
        }
        return std::nullopt;
    }

    static std::vector<std::shared_ptr<Position>>
    findVictims(World& world, const std::shared_ptr<ComponentMap<Position>>& posStorage,
                Entity executorEntity, const std::shared_ptr<Position>& executorPos) {
        std::vector<std::shared_ptr<Position>> victims;

        for (auto const& [entity, pos] : *posStorage) {
            if (entity == executorEntity) {
                continue;
            }
            if (!world.get_component<InhabitantTag>(entity) ||
                world.get_component<TileTag>(entity)) {
                continue;
            }
            if (pos->x == executorPos->x && pos->y == executorPos->y) {
                victims.push_back(pos);
                world.add_component<EventExpulsed>(entity, EventExpulsed{});
            }
        }
        return victims;
    }

    static void move_forward(const std::shared_ptr<Position>& pos, Orientation::Direction direction,
                             int mapWidth, int mapHeight) {
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
