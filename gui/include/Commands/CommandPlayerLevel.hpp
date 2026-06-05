/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandPlayerLevel.hpp
*/
#ifndef ZAPPY_COMMANDPLAYERLEVEL_HPP
#define ZAPPY_COMMANDPLAYERLEVEL_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include <algorithm>
#include <sstream>

namespace zappy {
class CommandPlayerLevel : public ACommand {
  public:
    CommandPlayerLevel() = default;
    ~CommandPlayerLevel() override = default;

    /**
     * @brief Handles the "plv" command, updating a player's level.
     * @param args The arguments for the command, expected to be "#n L" or "n L"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int playerId, level;

        if (!(iss >> playerId >> level)) {
            return;
        }

        auto levelStorage = world.get_storage<Level>();
        if (!levelStorage) {
            return;
        }

        for (auto& [entity, levelComp] : *levelStorage) {
            if (entity.id() == (uint32_t)playerId) {
                levelComp->level = level;
                std::cout << "Protocol: Player #" << playerId << " level updated to " << level
                          << std::endl;
                break;
            }
        }
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDPLAYERLEVEL_HPP
