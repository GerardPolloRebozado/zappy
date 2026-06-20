#ifndef ZAPPY_COMMANDEGGDEATH_HPP
#define ZAPPY_COMMANDEGGDEATH_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

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
            log_error("Protocol: failed to parse egg death args: " + args);
            return;
        }

        auto storage = world.get_storage<Egg>();
        if (storage) {
            for (auto const& [entity, egg] : *storage) {
                if (egg->id == eggId) {
                    std::string teamStr = "";
                    auto teamComp = world.get_component<TeamName>(entity);
                    if (teamComp) {
                        teamStr = teamComp->_team_name;
                    }
                    if (_chatLogs) {
                        _chatLogs->addChatLog("Egg #" + std::to_string(eggId) + " spoiled/died.",
                                              "DEATH", teamStr);
                    }
                    world.despawn(entity);
                    log_info("Protocol: Egg #" + std::to_string(eggId) + " died");
                    break;
                }
            }
        }
    }
};
} // namespace zappy
#endif