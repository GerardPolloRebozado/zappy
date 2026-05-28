#include "raylib-cpp.hpp"
#include <iostream>
#include <string>
#include <vector>

void print_usage() {
    std::cout << "USAGE: ./zappy_gui -p port -h machine" << std::endl << std::endl;
    std::cout << "option description" << std::endl;
    std::cout << "-p port port number" << std::endl;
    std::cout << "-h machine machine name" << std::endl;
}

int main(int argc, char** argv) {
    std::string port;
    std::string machine;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            print_usage();
            return 0;
        } else if (arg == "-p" && i + 1 < argc) {
            port = argv[++i];
        } else if (arg == "-h" && i + 1 < argc) {
            machine = argv[++i];
        } else {
            print_usage();
            return 84;
        }
    }

    if (port.empty() || machine.empty()) {
        print_usage();
        return 84;
    }

    std::cout << "Port: " << port << std::endl;
    std::cout << "Machine: " << machine << std::endl;

    try {
        raylib::Window window(1280, 720, "Zappy GUI");
        SetTargetFPS(60);

        while (!window.ShouldClose()) {
            BeginDrawing();
            window.ClearBackground(BLACK);
            EndDrawing();
        }
    } catch (const raylib::RaylibException& e) {
        std::cerr << "Raylib error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
