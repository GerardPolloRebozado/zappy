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
        void execute(const std::string&args, Register&registry) override
        {
            std::istringstream iss(args);
            int width, height;

            if (iss >> width >> height) {
                std::cout << "Protocol: Map size update " << width << "x" << height << std::endl;
            }
        };
    };
} // zappy

#endif //ZAPPY_COMMANDMAPSIZE_HPP
