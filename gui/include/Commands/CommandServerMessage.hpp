/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandServerMessage.hpp
*/
#ifndef ZAPPY_COMMANDSERVERMESSAGE_HPP
#define ZAPPY_COMMANDSERVERMESSAGE_HPP

#include "ACommand.hpp"
#include "Commands/CommandGetMapEvent.hpp"
#include "Logging/Logger.hpp"
#include <sstream>
#include <string>

namespace zappy {
class CommandServerMessage : public ACommand {
  public:
    CommandServerMessage() = default;
    ~CommandServerMessage() override = default;

    void execute(const std::string& args, World& world) override {
        log_info("Server message: " + args);

        std::istringstream iss(args);
        std::string phase;
        std::string eventName;

        if (!(iss >> phase >> eventName)) {
            return;
        }

        if (phase == "event_start") {
            updateMapEventState(world, eventName);
            if (_chatLogs) {
                _chatLogs->addChatLog("Celestial anomaly started: " + eventName, "EVENT");
            }
        } else if (phase == "event_end") {
            updateMapEventState(world, "none");
            if (_chatLogs) {
                _chatLogs->addChatLog("Celestial anomaly ended: " + eventName, "EVENT");
            }
        }
    }
};
} // namespace zappy
#endif
