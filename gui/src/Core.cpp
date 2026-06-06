/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** Core.cpp
*/

#include "Core.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
#include <iostream>

namespace zappy {

Core::Core(int port, const std::string& host) : _port(port), _host(host) {
    _window = std::make_unique<raylib::Window>(1280, 720, "Zappy GUI");
    SetTargetFPS(60);

    if (!_network.connect(_host, _port)) {
        std::cerr << "Core: Failed to connect to " << _host << ":" << _port << std::endl;
    } else {
        std::cout << "Core: Connected to " << _host << ":" << _port << std::endl;
    }

    _setupTestingData();
    _renderSystem.centerCamera(10, 10);
}

void Core::run() {
    while (!_window->ShouldClose()) {
        _update();
        _render();
    }
}

void Core::_update() {
    _network.update(_world);
    if (_network.isConnected()) {
        _renderSystem.update(_world, _window->GetFrameTime());
    }
}

void Core::_render() {
    _window->BeginDrawing();
    _window->ClearBackground(BLACK);

    if (!_network.isConnected()) {
        DrawText("Disconnected from server", 10, 10, 20, RED);
    } else {
        DrawText("Connected to server", 10, 10, 20, GREEN);
        _renderSystem.render(_world);
    }

    _window->EndDrawing();
}

void Core::_setupTestingData() {
    // Hardcoded testing data
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            auto tile = _world.spawn();
            _world.add_component(tile, Position{x, y});
            _world.add_component(tile, TerrainType{TerrainType::GRASS});
            _world.add_component(tile, Inventory{x + y, x, y, 0, 0, 0, 0});
        }
    }

    auto p1 = _world.spawn();
    _world.add_component(p1, Position{2, 2});
    _world.add_component(p1, Orientation{Orientation::N});
    _world.add_component(p1, Level{1});
    _world.add_component(p1, TeamName{"Team Alpha"});

    auto p2 = _world.spawn();
    _world.add_component(p2, Position{5, 5});
    _world.add_component(p2, Orientation{Orientation::E});
    _world.add_component(p2, Level{4});
    _world.add_component(p2, TeamName{"Team Beta"});
}

} // namespace zappy
