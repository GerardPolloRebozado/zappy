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
        void execute(const std::string&args, Register&registry) override
        {
            std::istringstream iss(args);
            int x, y, q0, q1, q2, q3, q4, q5, q6;

            if (iss >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6) {
                int tileEntity = registry.createEntity();
                registry._tileTags[tileEntity] = TileTag{};
                registry._positions[tileEntity] = Position{x, y};
                registry._inventories[tileEntity] = Inventory{q0, q1, q2, q3, q4, q5, q6};
                std::cout << "Tile created at (" << x << "," << y << ") with " << q0 << " food." << std::endl;
            }
        };
    };
} // zappy

#endif //ZAPPY_COMMANDTILECONTENT_HPP
