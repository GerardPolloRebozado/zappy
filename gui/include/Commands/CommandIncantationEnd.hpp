/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandIncantationEnd.hpp
*/
#ifndef ZAPPY_COMMANDINCANTATIONEND_HPP
#define ZAPPY_COMMANDINCANTATIONEND_HPP

#include "ACommand.hpp"
#include "Components/ComponentIncantationEffect.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Graphics/AssetManager.hpp"
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

        try {
            auto& sound = AssetManager::getInstance().getSound("incantation_end");
            if (!sound.IsPlaying()) {
                sound.Play();
            }
        } catch (const raylib::RaylibException& e) {
            log_error("Failed to play incantation_end sound: " + std::string(e.what()));
        }

        auto eventEntity = world.spawn();
        world.add_component(eventEntity, Position{x, y});
        world.add_component(eventEntity, EventIncantationEnd{result});
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDINCANTATIONEND_HPP