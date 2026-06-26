/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandMapEvent.hpp
*/
#ifndef ZAPPY_COMMANDMAPEVENT_HPP
#define ZAPPY_COMMANDMAPEVENT_HPP

#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Logging/Logger.hpp"
#include <sstream>
#include <string>

namespace zappy {

inline void updateMapEventState(World& world, const std::string& name, int centerX = -1,
                                int centerY = -1) {
    const bool active = (name != "none");

    auto storage = world.get_storage<MapEvent>();
    bool found = false;
    if (storage) {
        for (auto const& [entity, mapEvent] : *storage) {
            mapEvent->name = name;
            mapEvent->centerX = centerX;
            mapEvent->centerY = centerY;
            mapEvent->active = active;
            found = true;
            break;
        }
    }

    if (!found) {
        Entity eventEntity = world.spawn();
        world.add_component<MapEvent>(eventEntity, {name, centerX, centerY, active});
    }
}

class CommandMapEvent : public ACommand {
  public:
    CommandMapEvent() = default;
    ~CommandMapEvent() override = default;

    /**
     * @brief Handles server responses for "gev" (get map event) and "mev" (trigger map event).
     *
     * Response formats:
     * - gev: "none" or "<name>" or "gravity_well X Y"
     * - mev: "<name>" on successful trigger
     */
    void execute(const std::string& args, World& world) override {
        std::istringstream iss(args);
        std::string name;

        if (!(iss >> name)) {
            log_error("Protocol: failed to parse map event args: " + args);
            return;
        }

        int centerX = -1;
        int centerY = -1;
        if (name == "gravity_well") {
            iss >> centerX >> centerY;
        }

        updateMapEventState(world, name, centerX, centerY);

        if (_chatLogs) {
            if (name == "none") {
                _chatLogs->addChatLog("No celestial anomaly active", "EVENT");
            } else if (name == "gravity_well" && centerX >= 0 && centerY >= 0) {
                _chatLogs->addChatLog("Celestial anomaly: " + name + " at [" +
                                          std::to_string(centerX) + ", " + std::to_string(centerY) +
                                          "]",
                                      "EVENT");
            } else {
                _chatLogs->addChatLog("Celestial anomaly: " + name, "EVENT");
            }
        }

        log_info("Protocol: Map event update [" + args + "]");
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDMAPEVENT_HPP
