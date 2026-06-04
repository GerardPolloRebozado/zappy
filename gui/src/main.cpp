#include "NetworkManager.hpp"
#include "Systems/RenderSystem.hpp"
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
    zappy::World registry;
    zappy::RenderSystem renderSys;

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
        zappy::NetworkManager network;

        if (!network.connect(machine, port)) {
            std::cerr << "Failed to connect to " << machine << ":" << port << "\n";
        } else {
            std::cout << "Connected to " << machine << ":" << port << "\n";
        }

        raylib::Window window(1280, 720, "Zappy GUI");
        SetTargetFPS(60);

        // Map will be populated via network (mct command)
        // renderSys.camera position will be adjusted once msz is received

        while (!window.ShouldClose()) {

            network.update(registry);

            window.BeginDrawing();
            window.ClearBackground(BLACK);

            if (!network.isConnected()) {
                DrawText("Disconnected from server", 10, 10, 20, RED);
            } else {
                DrawText("Connected to server", 10, 10, 20, GREEN);
                renderSys.update(registry);
            }

            window.EndDrawing();
        }
    } catch (const std::invalid_argument&) {
        std::cerr << "Error: Invalid port number.\n";
        return 84;
    } catch (const raylib::RaylibException& e) {
        std::cerr << "Raylib error: " << e.what() << "\n";
        return 84;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 84;
    }

    return 0;
}
