/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTileContent.hpp
*/
#ifndef ZAPPY_COMMANDTILECONTENT_HPP
#define ZAPPY_COMMANDTILECONTENT_HPP
#include "ACommand.hpp"
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

        world.add_component<Inventory>(tileEntity, {q0, q1, q2, q3, q4, q5, q6});

        // terrain type
        int t_type;
        if (iss >> t_type) {
            world.add_component<TerrainType>(tileEntity, {static_cast<TerrainType::Type>(t_type)});
        }

        log_info("Protocol: Tile (" + std::to_string(x) + ", " + std::to_string(y) +
                    ") content updated");
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDTILECONTENT_HPP
