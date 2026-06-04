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

namespace zappy {
class CommandTileContent : public ACommand {
  public:
    CommandTileContent() = default;

    /**
     * @brief Handles the "bct" command, which provides the content of a specific tile on the map.
     * Parses the tile coordinates, inventory quantities, and terrain type from the command
     * arguments. Updates the world with the tile's position, inventory, and terrain type. If an
     * entity for the tile doesn't exist, it creates one and assigns the appropriate components.
     * @param args The arguments for the command, expected to be in the format "x y q0 q1 q2 q3 q4
     * q5 q6 t_type", where (x, y) are the tile coordinates, q0-q6 are the quantities of each
     * resource, and t_type is the terrain type identifier.
     * @param world The world containing the application state, where the tile's content will be
     * updated
     */
    void execute(const std::string& args, World& world) override {
        std::istringstream iss(args);
        int x, y, q0, q1, q2, q3, q4, q5, q6, t_type;
        if (!(iss >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6 >> t_type)) {
            return;
        }

        Entity tileEntity(0, 0);
        bool found = false;
        auto posStorage = world.get_storage<Position>();
        if (posStorage) {
            for (auto const& [ent, pos] : *posStorage) {
                if (pos->x == x && pos->y == y) {
                    if (world.get_component<TerrainType>(ent)) {
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
        world.add_component<TerrainType>(tileEntity, {(zappy::TerrainType::Type)t_type});

        if (x == 0 && y == 0) {
            std::cout << "Debug: Received tile(0,0) terrain type: " << t_type << std::endl;
        }
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDTILECONTENT_HPP
