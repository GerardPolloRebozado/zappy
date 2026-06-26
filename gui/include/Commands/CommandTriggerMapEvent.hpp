/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTriggerMapEvent.hpp
*/
#ifndef ZAPPY_COMMANDTRIGGERMAPEVENT_HPP
#define ZAPPY_COMMANDTRIGGERMAPEVENT_HPP

#include "ACommand.hpp"
#include "Commands/CommandGetMapEvent.hpp"
#include "Logging/Logger.hpp"
#include <sstream>
#include <string>

namespace zappy {

/**
 * @brief Handles server responses for "mev" (manual map event trigger).
 *
 * Lifecycle state (active/inactive) is driven by smg event_start/event_end broadcasts.
 * This handler only records gravity well coordinates and logs them in chat.
 */
class CommandTriggerMapEvent : public ACommand {
  public:
    CommandTriggerMapEvent() = default;
    ~CommandTriggerMapEvent() override = default;

    void execute(const std::string& args, World& world) override {
        std::istringstream iss(args);
        std::string name;

        if (!(iss >> name)) {
            log_error("Protocol: failed to parse map event trigger args: " + args);
            return;
        }

        int centerX = -1;
        int centerY = -1;
        if (name == "gravity_well") {
            iss >> centerX >> centerY;
        }

        if (name == "gravity_well" && centerX >= 0 && centerY >= 0) {
            updateMapEventCoords(world, centerX, centerY);
            if (_chatLogs) {
                _chatLogs->addChatLog("Celestial anomaly: " + name + " at [" +
                                          std::to_string(centerX) + ", " + std::to_string(centerY) +
                                          "]",
                                      "EVENT");
            }
        }

        log_info("Protocol: Map event triggered [" + args + "]");
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDTRIGGERMAPEVENT_HPP
