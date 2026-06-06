/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** main.cpp
*/

#include "Core.hpp"
#include <iostream>
#include <string>

void print_usage() {
    std::cout << "USAGE: ./zappy_gui -p port -h machine\n\n";
    std::cout << "option description\n";
    std::cout << "-p port port number\n";
    std::cout << "-h machine machine name\n";
}

int main(int argc, char** argv) {
    std::string portStr;
    std::string machine;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            print_usage();
            return 0;
        } else if (arg == "-p" && i + 1 < argc) {
            portStr = argv[++i];
        } else if (arg == "-h" && i + 1 < argc) {
            machine = argv[++i];
        } else {
            print_usage();
            return 84;
        }
    }

    if (portStr.empty() || machine.empty()) {
        print_usage();
        return 84;
    }

    try {
        int port = std::stoi(portStr);
        zappy::Core core(port, machine);
        core.run();
    } catch (const std::invalid_argument&) {
        std::cerr << "Error: Invalid port number.\n";
        return 84;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 84;
    }

    return 0;
}
