/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CommandMapSize.hpp
*/
#ifndef ZAPPY_COMMANDMAPSIZE_HPP
#define ZAPPY_COMMANDMAPSIZE_HPP
#include "ACommand.hpp"
namespace zappy {
    class CommandMapSize : public ACommand {
    public:
        CommandMapSize() = default;
        /**
         * @brief Handles the "msz" command, which provides the dimensions of the map.
         * Parses the width and height from the command arguments and updates the registry.
         * If an entity for the map size doesn't exist, it creates one and assigns the Size component.
         * @param args The arguments for the command, expected to be in the format "width height".
         * @param registry The registry containing the application state, where the map size will
         */
        void execute(const std::string&args, Register&registry) override
        {
            std::istringstream iss(args);
            int width, height;
            if (!(iss >> width >> height)) {
                return;
            }

            int tileEntity = -1;
            for (auto const& [ent, size] : registry._sizes) {
                if (size.width == width && size.height == height) {
                    tileEntity = ent;
                    break;
                }
            }

            if (tileEntity == -1) {
                tileEntity = registry.createEntity();
                registry._sizes[tileEntity] = {width, height};
                registry._mapTags[tileEntity] = {};
            }
            std::cout << "Protocol: Map size update [" << args << "]" << std::endl;

        }
    };
} // zappy

#endif //ZAPPY_COMMANDMAPSIZE_HPP
