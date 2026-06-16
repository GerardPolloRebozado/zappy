/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandEggConnection.hpp
*/
#ifndef ZAPPY_COMMANDEGGCONNECTION_HPP
#define ZAPPY_COMMANDEGGCONNECTION_HPP

#include "ACommand.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <string>

namespace zappy {
class CommandEggConnection : public ACommand {
  public:
    CommandEggConnection() = default;
    ~CommandEggConnection() override = default;

    /**
     * @brief Handles the "ebo" command, player connection for an egg (egg vanishes).
     * @param args The arguments for the command: "#e"
     * @param world The ECS World containing the application state
     */
    void execute(const std::string& args, World& world) override {
        std::string cleanArgs = args;
        cleanArgs.erase(std::remove(cleanArgs.begin(), cleanArgs.end(), '#'), cleanArgs.end());

        std::istringstream iss(cleanArgs);
        int eggId;

        if (!(iss >> eggId)) {
            log_error("Protocol: failed to parse egg connection args: " + args);
            return;
        }

        auto storage = world.get_storage<Egg>();
        if (storage) {
            for (auto const& [entity, egg] : *storage) {
                if (egg->id == eggId) {
                    auto posComponent = world.get_component<Position>(entity);
                    // TODO: i think this should _not_ be done by the actual command
                    if (posComponent) {
                        auto confettiEntity = world.spawn();
                        world.add_component<Position>(confettiEntity, *posComponent);

                        ComponentParticleEmitter emitter;
                        emitter.isPlaying = true;
                        emitter.loop = false;
                        emitter.duration = 0.1f;
                        emitter.emitRate = 500.0f;
                        emitter.minLifetime = 0.5f;
                        emitter.maxLifetime = 1.5f;
                        emitter.minSize = 0.05f;
                        emitter.maxSize = 0.15f;
                        emitter.minVelocity = raylib::Vector3(-3.0f, 4.0f, -3.0f);
                        emitter.maxVelocity = raylib::Vector3(3.0f, 8.0f, 3.0f);
                        emitter.colorPalette = {raylib::Color::Red(),     raylib::Color::Green(),
                                                raylib::Color::Blue(),    raylib::Color::Yellow(),
                                                raylib::Color::Magenta(), raylib::Color::SkyBlue(),
                                                raylib::Color::Orange()};

                        world.add_component(confettiEntity, emitter);
                    }
                    world.despawn(entity);
                    log_info("Protocol: Player connected to egg #" + std::to_string(eggId) +
                             " (egg removed)");
                    break;
                }
            }
        }
    }
};
} // namespace zappy
#endif
