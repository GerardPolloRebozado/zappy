/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandEggDeath.hpp
*/
#ifndef ZAPPY_COMMANDEGGDEATH_HPP
#define ZAPPY_COMMANDEGGDEATH_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include <algorithm>
#include <sstream>

namespace zappy {
class CommandEggDeath : public ACommand {
  public:
    CommandEggDeath() = default;
    ~CommandEggDeath() override = default;

    /**
     * @brief Handles the "edi" command, indicating the death of an egg.
     * @param args The arguments for the command: "#e"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int eggId;

        if (!(iss >> eggId)) {
            return;
        }

        auto storage = world.get_storage<Egg>();
        if (storage) {
            for (auto const& [entity, egg] : *storage) {
                if (egg->id == eggId) {
                    world.despawn(entity);
                    std::cout << "Protocol: Egg #" << eggId << " died" << std::endl;
                    break;
                }
            }
        }
    }
};
} // namespace zappy
#endif
