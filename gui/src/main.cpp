#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
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

        // Hardcoded testing data
        for (int x = 0; x < 10; ++x) {
            for (int y = 0; y < 10; ++y) {
                auto tile = registry.spawn();
                registry.add_component(tile, zappy::Position{x, y});
                registry.add_component(tile, zappy::TerrainType{zappy::TerrainType::GRASS});
                registry.add_component(tile, zappy::Inventory{x + y, x, y, 0, 0, 0, 0});
            }
        }

        auto p1 = registry.spawn();
        registry.add_component(p1, zappy::Position{2, 2});
        registry.add_component(p1, zappy::Orientation{zappy::Orientation::N});
        registry.add_component(p1, zappy::Level{1});
        registry.add_component(p1, zappy::TeamName{"Team Alpha"});

        auto p2 = registry.spawn();
        registry.add_component(p2, zappy::Position{5, 5});
        registry.add_component(p2, zappy::Orientation{zappy::Orientation::E});
        registry.add_component(p2, zappy::Level{4});
        registry.add_component(p2, zappy::TeamName{"Team Beta"});

        renderSys.centerCamera(10, 10);

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
