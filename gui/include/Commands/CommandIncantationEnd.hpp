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
#include <sstream>

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
            return;
        }

        std::cout << "Protocol: Incantation at (" << x << ", " << y
                  << ") ended with result: " << (result ? "Success" : "Failure") << std::endl;
    }
};
} // namespace zappy

#endif // ZAPPY_COMMANDINCANTATIONEND_HPP
