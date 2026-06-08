/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTimeUpdate.hpp
*/
#ifndef ZAPPY_COMMANDTIMEUPDATE_HPP
#define ZAPPY_COMMANDTIMEUPDATE_HPP

#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Logging/Logger.hpp"
#include <sstream>
#include <string>

namespace zappy {
class CommandTimeUpdate : public ACommand {
  public:
    CommandTimeUpdate() = default;
    ~CommandTimeUpdate() override = default;

    /**
     * @brief Handles the "sgt" and "sst" commands.
     * @param args The arguments for the command: "T"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::istringstream iss(args);
        int frequency;

        if (!(iss >> frequency)) {
            ZAPPY_LOG_E("Protocol: failed to parse time update args: " + args);
            return;
        }

        auto storage = world.get_storage<TimeUnit>();
        bool found = false;
        if (storage) {
            for (auto const& [entity, timeUnit] : *storage) {
                timeUnit->frequency = frequency;
                found = true;
                break;
            }
        }

        if (!found) {
            Entity timeEntity = world.spawn();
            world.add_component<TimeUnit>(timeEntity, {frequency});
        }

        ZAPPY_LOG_I("Protocol: Time unit updated to " + std::to_string(frequency));
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDTIMEUPDATE_HPP
