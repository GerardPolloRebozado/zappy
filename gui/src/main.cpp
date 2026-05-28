#include "NetworkManager.hpp"
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

    int port = std::stoi(portStr);
    zappy::NetworkManager network;

    if (!network.connect(machine, port)) {
        std::cerr << "Failed to connect to " << machine << ":" << port << std::endl;
    } else {
        std::cout << "Connected to " << machine << ":" << port << std::endl;
    }

    try {
        raylib::Window window(1280, 720, "Zappy GUI");
        SetTargetFPS(60);

        while (!window.ShouldClose()) {
            network.update();

            BeginDrawing();
            window.ClearBackground(BLACK);

            if (!network.isConnected()) {
                DrawText("Disconnected from server", 10, 10, 20, RED);
            } else {
                DrawText("Connected to server", 10, 10, 20, GREEN);
            }

            EndDrawing();
        }
    } catch (const raylib::RaylibException& e) {
        std::cerr << "Raylib error: " << e.what() << std::endl;
        return 84;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
