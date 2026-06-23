/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTileContent.hpp
*/
#ifndef ZAPPY_COMMANDTILECONTENT_HPP
#define ZAPPY_COMMANDTILECONTENT_HPP
#include "ACommand.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Components/ComponentTile.hpp"
#include "Logging/Logger.hpp"
#include <string>

namespace zappy {
class CommandTileContent : public ACommand {
  public:
    CommandTileContent() = default;

    /**
     * @brief Handles the "bct" command, which provides the content of a specific tile on the map.
     * Parses the tile coordinates and inventory quantities from the command arguments.
     * Support both protocol standard (9 args) and project extension (10 args with terrain type).
     * @param args The arguments for the command, expected to be "x y q0 q1 q2 q3 q4 q5 q6 [t_type]"
     * @param world The world containing the application state.
     */
    void execute(const std::string& args, World& world) override {
        std::istringstream iss(args);
        int x, y, q0, q1, q2, q3, q4, q5, q6;
        if (!(iss >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6)) {
            log_error("Protocol: failed to parse tile content args: " + args);
            return;
        }

        Entity tileEntity(0, 0);
        bool found = false;
        auto posStorage = world.get_storage<Position>();
        if (posStorage) {
            for (auto const& [ent, pos] : *posStorage) {
                if (pos->x == x && pos->y == y) {
                    if (world.get_component<TileTag>(ent)) {
                        tileEntity = ent;
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found) {
            tileEntity = world.spawn();
            world.add_component<Position>(tileEntity, {x, y});
            world.add_component<TileTag>(tileEntity, TileTag{});
        }

        auto oldInv = world.get_component<Inventory>(tileEntity);
        int old_q0 = oldInv ? oldInv->food : 0;
        int old_q1 = oldInv ? oldInv->linemate : 0;
        int old_q2 = oldInv ? oldInv->deraumere : 0;
        int old_q3 = oldInv ? oldInv->sibur : 0;
        int old_q4 = oldInv ? oldInv->mendiane : 0;
        int old_q5 = oldInv ? oldInv->phiras : 0;
        int old_q6 = oldInv ? oldInv->thystame : 0;

        world.add_component<Inventory>(tileEntity, {std::min(old_q0, q0), std::min(old_q1, q1),
                                                    std::min(old_q2, q2), std::min(old_q3, q3),
                                                    std::min(old_q4, q4), std::min(old_q5, q5),
                                                    std::min(old_q6, q6)});

        auto spawnFallingResource = [&](ResourceType resId, int delta) {
            for (int i = 0; i < delta; ++i) {
                Entity res = world.spawn();
                world.add_component<AnimatedResource>(res, {resId, true});
                world.add_component<Position>(res, {x, y});
                world.add_component<Animation>(res, {"", 0.0f, 60.0f, 1.0f, true});
                world.add_component<MovementInterpolation3D>(
                    res, {static_cast<float>(x), static_cast<float>(y),
                          20.0f + (i * 2.0f), // Start high in the sky
                          true, 2.01f});
            }
        };

        if (q0 > old_q0) {
            spawnFallingResource(ResourceType::FOOD, q0 - old_q0);
        }
        if (q1 > old_q1) {
            spawnFallingResource(ResourceType::LINEMATE, q1 - old_q1);
        }
        if (q2 > old_q2) {
            spawnFallingResource(ResourceType::DERAUMERE, q2 - old_q2);
        }
        if (q3 > old_q3) {
            spawnFallingResource(ResourceType::SIBUR, q3 - old_q3);
        }
        if (q4 > old_q4) {
            spawnFallingResource(ResourceType::MENDIANE, q4 - old_q4);
        }
        if (q5 > old_q5) {
            spawnFallingResource(ResourceType::PHIRAS, q5 - old_q5);
        }
        if (q6 > old_q6) {
            spawnFallingResource(ResourceType::THYSTAME, q6 - old_q6);
        }

        // terrain type
        int t_type;
        if (iss >> t_type) {
            auto newType = static_cast<TerrainType::Type>(t_type);
            world.add_component<TerrainType>(tileEntity, {newType});

            if (newType == TerrainType::OBSIDIAN_BARRENS) {
                if (!world.get_component<ComponentParticleEmitter>(tileEntity)) {
                    ComponentParticleEmitter emitter;
                    emitter.loop = true;
                    emitter.isPlaying = true;
                    emitter.emitRate = 8.0f;                       // Subtle simmer
                    emitter.offset = raylib::Vector3(0, 0.05f, 0); // just above the floor
                    emitter.spawnVolumeMin =
                        raylib::Vector3(-0.45f, 0.0f, -0.45f); // Spread over the tile
                    emitter.spawnVolumeMax = raylib::Vector3(0.45f, 0.1f, 0.45f);
                    emitter.minLifetime = 1.0f;
                    emitter.maxLifetime = 2.5f;
                    emitter.minSize = 0.03f;
                    emitter.maxSize = 0.08f;
                    emitter.minVelocity = raylib::Vector3(-0.15f, 0.3f, -0.15f);
                    emitter.maxVelocity =
                        raylib::Vector3(0.15f, 0.8f, 0.15f); // rising smoke/embers
                    // Mix between smoke and fire particles
                    emitter.colorPalette = {
                        raylib::Color{255, 100, 0, 200}, // fire orange
                        raylib::Color{255, 50, 0, 200},  // red-hot
                        raylib::Color{50, 50, 50, 150},  // dark smoke
                        raylib::Color{20, 20, 20, 150}   // obsidian ash
                    };
                    world.add_component<ComponentParticleEmitter>(tileEntity, emitter);
                }
            } else {
                if (world.get_component<ComponentParticleEmitter>(tileEntity)) {
                    world.remove_component<ComponentParticleEmitter>(tileEntity);
                }
            }
        }

        log_info("Protocol: Tile (" + std::to_string(x) + ", " + std::to_string(y) +
                 ") content updated");
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDTILECONTENT_HPP
