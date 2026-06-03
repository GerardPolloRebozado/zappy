/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandTileContent.hpp
*/
#ifndef ZAPPY_COMMANDTILECONTENT_HPP
#define ZAPPY_COMMANDTILECONTENT_HPP
#include "ACommand.hpp"

namespace zappy {
    class CommandTileContent : public ACommand{
    public:
        CommandTileContent() = default;

        /**
         * @brief Handles the "bct" command, which provides the content of a specific tile on the map.
         * Parses the tile coordinates, inventory quantities, and terrain type from the command arguments.
         * Updates the registry with the tile's position, inventory, and terrain type. If an entity for the tile doesn't exist, it creates one and assigns the appropriate components.
         * @param args The arguments for the command, expected to be in the format "x y q0 q1 q2 q3 q4 q5 q6 t_type", where (x, y) are the tile coordinates, q0-q6 are the quantities of each resource, and t_type is the terrain type identifier.
         * @param registry The registry containing the application state, where the tile's content will be updated
         */
        void execute(const std::string&args, Register&registry) override
        {
            std::istringstream iss(args);
            int x, y, q0, q1, q2, q3, q4, q5, q6, t_type;
            if (!(iss >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6 >> t_type)) {
                return;
            }

            int tileEntity = -1;
            for (auto const& [ent, pos] : registry._positions) {
                if (pos.x == x && pos.y == y && registry._terrainTypes.count(ent)) {
                    tileEntity = ent;
                    break;
                }
            }

            if (tileEntity == -1) {
                tileEntity = registry.createEntity();
                registry._positions[tileEntity] = {x, y};
                registry._tileTags[tileEntity] = {};
            }

            registry._inventories[tileEntity] = {q0, q1, q2, q3, q4, q5, q6};
            registry._terrainTypes[tileEntity] = {(zappy::TerrainType::Type)t_type};

            if (x == 0 && y == 0) {
                std::cout << "Debug: Received tile(0,0) terrain type: " << t_type << std::endl;
            }
        }
    };
} // zappy

#endif //ZAPPY_COMMANDTILECONTENT_HPP
