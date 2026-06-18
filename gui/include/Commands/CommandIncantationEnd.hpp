/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandIncantationEnd.hpp
*/
#ifndef ZAPPY_COMMANDINCANTATIONEND_HPP
#define ZAPPY_COMMANDINCANTATIONEND_HPP

#include "ACommand.hpp"
#include "Components/ComponentShared.hpp"
#include "Logging/Logger.hpp"
#include <sstream>
#include <string>

namespace zappy {
class CommandIncantationEnd : public ACommand {
  public:
    CommandIncantationEnd() = default;
    ~CommandIncantationEnd() override = default;

    /**
     * @brief Handles the "pie" command, indicating the end of an incantation.
     * @param args The arguments for the command: "X Y R"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::istringstream iss(args);
        int x, y, result;

        if (!(iss >> x >> y >> result)) {
            log_error("Protocol: failed to parse incantation end args: " + args);
            return;
        }

        if (_chatLogs) {
            std::string resStr = result ? "succeeded!" : "failed.";
            _chatLogs->addChatLog("Incantation at (" + std::to_string(x) + "," + std::to_string(y) +
                                      ") " + resStr,
                                  "INFO");
        }

        log_info("Protocol: Incantation at (" + std::to_string(x) + ", " + std::to_string(y) +
                 ") ended with result: " + (result ? "Success" : "Failure"));
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDINCANTATIONEND_HPP