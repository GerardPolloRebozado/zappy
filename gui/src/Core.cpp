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
#include "Graphics/AssetManager.hpp"
#include "UI/UIButton.hpp"
#include "UI/UIHudPanel.hpp"
#include "UI/UIInput.hpp"
#include "UI/UIPanel.hpp"
#include "UI/UIScoreboardPanel.hpp"
#include "UI/UIText.hpp"
#include <iostream>

namespace zappy {

Core::Core(int port, const std::string& host) : _port(port), _host(host) {
    _window = std::make_unique<raylib::Window>(1280, 720, "Zappy GUI");
    _window->SetTargetFPS(60);

    // Prevent ESC from instantly closing the entire application
    _window->SetExitKey(0);

    // Load assets and hide default cursor so our custom cursor renders immediately
    AssetManager::getInstance().loadAll();
    raylib::Window::HideCursor();

    _appState = AppState::MENU;
    _setupMainMenu();
}

Core::~Core() { AssetManager::getInstance().unloadAll(); }

void Core::run() {
    while (!_window->ShouldClose() && !_shouldClose) {
        _update();
        _render();
    }
}

void Core::_update() {
    float dt = _window->GetFrameTime();
    _uiManager.update(dt);

    if (_appState == AppState::PLAYING) {
        if (raylib::Keyboard::IsKeyPressed(KEY_ESCAPE)) {
            _network.disconnect();
            _appState = AppState::MENU;
            _setupMainMenu();
            return;
        }

        _network.update(_world);
        if (_network.isConnected()) {
            _renderSystem.update(_world, dt);
        } else {
            // Server disconnected, go back to menu
            _appState = AppState::MENU;
            _setupMainMenu();
        }
    }
}

void Core::_render() {
    _window->BeginDrawing();
    _window->ClearBackground(BLACK);

    if (_appState == AppState::PLAYING) {
        _renderSystem.render(_world);
    }

    _uiManager.render();

    // Draw custom cursor on top of everything
    auto& am = AssetManager::getInstance();
    raylib::Texture2D& tex = raylib::Mouse::IsButtonDown(MOUSE_BUTTON_LEFT)
                                 ? am.getTexture("mouse_pressed")
                                 : am.getTexture("mouse");
    if (tex.id != 0) {
        tex.Draw(raylib::Vector2((float)raylib::Mouse::GetX(), (float)raylib::Mouse::GetY()), 0.0f,
                 3.0f, WHITE);
    }

    _window->EndDrawing();
}

void Core::_clearMenuUI() { _uiManager.clear(); }

void Core::_setupMainMenu() {
    _clearMenuUI();

    int cx = _window->GetWidth() / 2;
    int cy = _window->GetHeight() / 2;

    // Background panel
    _uiManager.addComponent(std::make_shared<UIPanel>(
        raylib::Rectangle{0, 0, (float)_window->GetWidth(), (float)_window->GetHeight()},
        raylib::Color(20, 20, 30, 255), 0));

    // Title
    _uiManager.addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 100, (float)cy - 200, 200, 50},
                                 "ZAPPY", 40, raylib::Color::RayWhite(), 1));

    // Play Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy - 50, 200, 50}, "Play",
        [this]() { this->_showConnectionOverlay(); }, 1));

    // Settings Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy + 20, 200, 50}, "Settings",
        [this]() { this->_setupSettingsMenu(); }, 1));

    // Quit Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy + 90, 200, 50}, "Quit",
        [this]() { this->_shouldClose = true; }, 1));
}

void Core::_setupSettingsMenu() {
    _clearMenuUI();

    int cx = _window->GetWidth() / 2;
    int cy = _window->GetHeight() / 2;

    // Background panel
    _uiManager.addComponent(std::make_shared<UIPanel>(
        raylib::Rectangle{0, 0, (float)_window->GetWidth(), (float)_window->GetHeight()},
        raylib::Color(20, 20, 30, 255), 0));

    // Title
    _uiManager.addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 100, (float)cy - 250, 200, 50},
                                 "SETTINGS", 40, raylib::Color::RayWhite(), 1, 4.0f));

    // Resolution 1280x720 Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy - 130, 200, 40}, "1280x720",
        [this]() {
            this->_window->SetSize(1280, 720);
            this->_setupSettingsMenu(); // Re-center UI
        },
        1));

    // Resolution 1920x1080 Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy - 70, 200, 40}, "1920x1080",
        [this]() {
            this->_window->SetSize(1920, 1080);
            this->_setupSettingsMenu(); // Re-center UI
        },
        1));

    // Fullscreen Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy - 10, 200, 40}, "Toggle Fullscreen",
        [this]() {
            this->_window->ToggleFullscreen();
            this->_setupSettingsMenu(); // Re-center UI
        },
        1));

    // Volume Controls (Placeholder)
    _uiManager.addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 100, (float)cy + 50, 200, 30},
                                 "Volume:", 20, raylib::Color::RayWhite(), 1));

    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy + 80, 90, 40}, "-",
        []() { std::cout << "Volume Down" << std::endl; }, 1));

    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy + 80, 90, 40}, "+",
        []() { std::cout << "Volume Up" << std::endl; }, 1));

    // Back Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy + 150, 200, 50}, "Back",
        [this]() { this->_setupMainMenu(); }, 1));
}

void Core::_showConnectionOverlay() {
    int cx = _window->GetWidth() / 2;
    int cy = _window->GetHeight() / 2;

    // Overlay Background
    _uiManager.addComponent(
        std::make_shared<UIPanel>(raylib::Rectangle{(float)cx - 150, (float)cy - 100, 300, 250},
                                  raylib::Color(40, 40, 50, 255), 10));

    // Host Input
    auto hostInput = std::make_shared<UIInput>(
        raylib::Rectangle{(float)cx - 100, (float)cy - 50, 200, 40}, _host, "Host...", 256, 11);
    _uiManager.addComponent(hostInput);

    // Port Input
    auto portInput =
        std::make_shared<UIInput>(raylib::Rectangle{(float)cx - 100, (float)cy, 200, 40},
                                  std::to_string(_port), "Port...", 10, 11);
    _uiManager.addComponent(portInput);

    // Connect Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 100, (float)cy + 60, 200, 40}, "Connect",
        [this, hostInput, portInput]() {
            std::string h = hostInput->getText();
            int p = 0;
            try {
                p = std::stoi(portInput->getText());
            } catch (...) {
                p = 0;
            }
            this->_connectToServer(h, p);
        },
        11));
}

void Core::_connectToServer(const std::string& host, int port) {
    if (_network.connect(host, port)) {
        std::cout << "Core: Connected to " << host << ":" << port << std::endl;
        _appState = AppState::PLAYING;
        _clearMenuUI();
        _setupTestingData();
        _setupGameUI();
        _renderSystem.centerCamera(10, 10);
    } else {
        std::cerr << "Core: Failed to connect to " << host << ":" << port << std::endl;
        int cx = _window->GetWidth() / 2;
        int cy = _window->GetHeight() / 2;
        _uiManager.addComponent(
            std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy + 110, 300, 30},
                                     "Connection Failed!", 20, raylib::Color::Red(), 12, 2.0f));
    }
}

void Core::_setupGameUI() {
    // Left Scoreboard
    _uiManager.addComponent(
        std::make_shared<UIScoreboardPanel>(raylib::Rectangle{10, 50, 240, 400}, _world, 10));

    // Right Tile HUD
    _uiManager.addComponent(std::make_shared<UIHudPanel>(
        raylib::Rectangle{(float)_window->GetWidth() - 250, 50, 240, 450}, _world, _renderSystem,
        10));
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
