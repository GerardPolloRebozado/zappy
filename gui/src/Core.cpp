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
#include "CoreErrors.hpp"
#include "Graphics/AssetManager.hpp"
#include "Logging/Logger.hpp"
#include "Network/NetworkErrors.hpp"
#include "UI/UIButton.hpp"
#include "UI/UIHudPanel.hpp"
#include "UI/UIInput.hpp"
#include "UI/UIPanel.hpp"
#include "UI/UIScoreboardPanel.hpp"
#include "UI/UIText.hpp"
#include "errors/IError.hpp"

namespace zappy {

Core::Core(int port, const std::string& host) : _port(port), _host(host) {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    _window = std::make_unique<raylib::Window>(1280, 720, "Zappy");
    _window->SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    // Prevent ESC from instantly closing the entire application
    _window->SetExitKey(0);

    _window->Maximize();

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

    if (_window->IsResized()) {
        if (_appState == AppState::MENU) {
            switch (_menuState) {
                case MenuState::MAIN:
                    _setupMainMenu();
                    break;
                case MenuState::SETTINGS:
                    _setupSettingsMenu();
                    break;
                case MenuState::CONNECTION:
                    _showConnectionOverlay();
                    break;
            }
        } else if (_appState == AppState::PLAYING) {
            _clearMenuUI();
            _setupGameUI();
        }
    }

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
    _menuState = MenuState::MAIN;
    _clearMenuUI();

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int cx = sw / 2;
    int cy = sh / 2;

    // Background panel
    _uiManager.addComponent(std::make_shared<UIPanel>(raylib::Rectangle{0, 0, (float)sw, (float)sh},
                                                      raylib::Color(15, 20, 40, 255), 0));

    // Title
    _uiManager.addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy - 250, 300, 80},
                                 "ZAPPY", 80, raylib::Color::RayWhite(), 1));

    // Play Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy - 50, 300, 60}, "Play",
        [this]() { this->_showConnectionOverlay(); }, 1));

    // Settings Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 30, 300, 60}, "Settings",
        [this]() { this->_setupSettingsMenu(); }, 1));

    // Quit Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 110, 300, 60}, "Quit",
        [this]() { this->_shouldClose = true; }, 1));
}

void Core::_setupSettingsMenu() {
    _menuState = MenuState::SETTINGS;
    _clearMenuUI();

    int cx = _window->GetWidth() / 2;
    int cy = _window->GetHeight() / 2;

    // Background panel
    _uiManager.addComponent(std::make_shared<UIPanel>(
        raylib::Rectangle{0, 0, (float)_window->GetWidth(), (float)_window->GetHeight()},
        raylib::Color(15, 20, 40, 255), 0));

    // Title
    _uiManager.addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy - 300, 300, 60},
                                 "SETTINGS", 60, raylib::Color::RayWhite(), 1, 4.0f));

    auto setResolution = [this](int w, int h) {
        int monitor = GetCurrentMonitor();
        int mw = GetMonitorWidth(monitor);
        int mh = GetMonitorHeight(monitor);
        if (w > mw) {
            w = mw;
        }
        if (h > mh) {
            h = mh;
        }

        if (this->_window->IsFullscreen()) {
            this->_window->ToggleFullscreen();
        }
        if (this->_window->IsMaximized()) {
            this->_window->Restore();
        }
        this->_window->SetSize(w, h);
        this->_window->SetPosition((mw - w) / 2, (mh - h) / 2);
        this->_setupSettingsMenu();
    };

    // 800x600 Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 160, (float)cy - 150, 150, 50}, "800x600",
        [setResolution]() { setResolution(800, 600); }, 1));

    // 1280x720 Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy - 150, 150, 50}, "1280x720",
        [setResolution]() { setResolution(1280, 720); }, 1));

    // 1600x900 Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 160, (float)cy - 80, 150, 50}, "1600x900",
        [setResolution]() { setResolution(1600, 900); }, 1));

    // 1920x1080 Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy - 80, 150, 50}, "1920x1080",
        [setResolution]() { setResolution(1920, 1080); }, 1));

    // Fullscreen Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy - 10, 300, 50}, "Toggle Fullscreen",
        [this]() {
            int monitor = GetCurrentMonitor();
            if (!this->_window->IsFullscreen()) {
                this->_window->SetSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                this->_window->ToggleFullscreen();
            } else {
                this->_window->ToggleFullscreen();
                this->_window->SetSize(1280, 720);
                this->_window->SetPosition((GetMonitorWidth(monitor) - 1280) / 2,
                                           (GetMonitorHeight(monitor) - 720) / 2);
            }
            this->_setupSettingsMenu(); // Re-center UI
        },
        1));

    // Volume Controls (Placeholder)
    _uiManager.addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy + 60, 300, 40},
                                 "Volume:", 30, raylib::Color::RayWhite(), 1));

    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 110, 140, 50}, "-",
        []() { log_debug("Volume Down"); }, 1));

    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy + 110, 140, 50}, "+",
        []() { log_debug("Volume Up"); }, 1));

    // Back Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 190, 300, 60}, "Back",
        [this]() { this->_setupMainMenu(); }, 1));
}

void Core::_showConnectionOverlay() {
    _menuState = MenuState::CONNECTION;
    _clearMenuUI();

    int cx = _window->GetWidth() / 2;
    int cy = _window->GetHeight() / 2;

    // Background panel
    _uiManager.addComponent(std::make_shared<UIPanel>(
        raylib::Rectangle{0, 0, (float)_window->GetWidth(), (float)_window->GetHeight()},
        raylib::Color(15, 20, 40, 255), 0));

    // Title
    _uiManager.addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy - 200, 300, 60},
                                 "CONNECT", 60, raylib::Color::RayWhite(), 1, 4.0f));

    // Host Input
    auto hostInput = std::make_shared<UIInput>(
        raylib::Rectangle{(float)cx - 150, (float)cy - 80, 300, 50}, _host, "Host...", 256, 1);
    _uiManager.addComponent(hostInput);

    // Port Input
    auto portInput =
        std::make_shared<UIInput>(raylib::Rectangle{(float)cx - 150, (float)cy - 10, 300, 50},
                                  std::to_string(_port), "Port...", 10, 1);
    _uiManager.addComponent(portInput);

    // Go Back Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 60, 140, 50}, "Go Back",
        [this]() { this->_setupMainMenu(); }, 1));

    // Connect Button
    _uiManager.addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy + 60, 140, 50}, "Connect",
        [this, hostInput, portInput]() {
            std::string h = hostInput->getText();
            int p = 0;
            try {
                p = parsePort(portInput->getText());
            } catch (const IError& e) {
                log_error(e.what());
            }
            this->_connectToServer(h, p);
        },
        1));
}

void Core::_connectToServer(const std::string& host, int port) {
    if (_network.connect(host, port)) {
        log_info("Core: Connected to " + host + ":" + std::to_string(port));
        _appState = AppState::PLAYING;
        _clearMenuUI();
        _setupTestingData();
        _setupGameUI();
        _renderSystem.centerCamera(10, 10);
    } else {
        log_error(ErrorNetwork("Failed to connect to " + host + ":" + std::to_string(port)).what());
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
        raylib::Rectangle{(float)_window->GetWidth() - 220, 50, 200, 400}, _world, _renderSystem,
        10));
}

void Core::_setupTestingData() {
    return;
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
