/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** Core.cpp
*/

#include "Color.hpp"
#include "Commands/FactoryCommands.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
#include "Core.hpp"
#include "CoreErrors.hpp"
#include "Graphics/AssetManager.hpp"
#include "Logging/Logger.hpp"
#include "Network/NetworkErrors.hpp"
#include "UI/UIButton.hpp"
#include "UI/UIChatPanel.hpp"
#include "UI/UIHudPanel.hpp"
#include "UI/UIInput.hpp"
#include "UI/UIManager.hpp"
#include "UI/UIPanel.hpp"
#include "UI/UIScoreboardPanel.hpp"
#include "UI/UISlider.hpp"
#include "UI/UIText.hpp"
#include "errors/IError.hpp"
#include "raylib.h"
#include <memory>

namespace zappy {

void Core::_clearMenuUI() { _uiManager->clear(); }

void Core::_setupMainMenu() {
    _menuState = MenuState::MAIN;
    _clearMenuUI();

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int cx = sw / 2;
    int cy = sh / 2;

    // Background panel
    _uiManager->addComponent(
        std::make_shared<UIPanel>(raylib::Rectangle{0, 0, (float)sw, (float)sh}, "menu_bg", 0));

    // Title
    _uiManager->addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy - 250, 300, 80},
                                 "ZAPPY", 80, raylib::Color::RayWhite(), 1, 1.5f, "HeaderFont"));

    // Play Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy - 50, 300, 60}, "Play",
        [this]() { this->_showConnectionOverlay(); }, "btn_normal", "btn_hover", "btn_pressed", 1));

    // Settings Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 30, 300, 60}, "Settings",
        [this]() { this->_setupSettingsMenu(); }, "btn_normal", "btn_hover", "btn_pressed", 1));

    // Quit Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 110, 300, 60}, "Quit",
        [this]() { this->_shouldClose = true; }, "btn_normal", "btn_hover", "btn_pressed", 1));
}

void Core::_setupSettingsMenu() {
    _menuState = MenuState::SETTINGS;
    _clearMenuUI();

    int cx = _window->GetWidth() / 2;
    int cy = _window->GetHeight() / 2;

    // Background panel
    _uiManager->addComponent(std::make_shared<UIPanel>(
        raylib::Rectangle{0, 0, (float)_window->GetWidth(), (float)_window->GetHeight()}, "menu_bg",
        0));

    // Title
    _uiManager->addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy - 300, 300, 60},
                                 "SETTINGS", 60, raylib::Color::RayWhite(), 1, 4.0f, "HeaderFont"));

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
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 160, (float)cy - 150, 150, 50}, "800x600",
        [setResolution]() { setResolution(800, 600); }, "btn_normal", "btn_hover", "btn_pressed",
        1));

    // 1280x720 Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy - 150, 150, 50}, "1280x720",
        [setResolution]() { setResolution(1280, 720); }, "btn_normal", "btn_hover", "btn_pressed",
        1));

    // 1600x900 Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 160, (float)cy - 80, 150, 50}, "1600x900",
        [setResolution]() { setResolution(1600, 900); }, "btn_normal", "btn_hover", "btn_pressed",
        1));

    // 1920x1080 Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy - 80, 150, 50}, "1920x1080",
        [setResolution]() { setResolution(1920, 1080); }, "btn_normal", "btn_hover", "btn_pressed",
        1));

    // Fullscreen Button
    _uiManager->addComponent(std::make_shared<UIButton>(
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
        "btn_normal", "btn_hover", "btn_pressed", 1));

    // Volume Controls (Placeholder)
    _uiManager->addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy + 60, 300, 40},
                                 "Volume:", 30, raylib::Color::RayWhite(), 1));

    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 110, 140, 50}, "-",
        [this]() {
            _musicSystem.volumeDown(); /*log_debug("Volume Down");*/
        },
        1));

    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx + 10, (float)cy + 110, 140, 50}, "+",
        [this]() {
            _musicSystem.volumeUp(); /*log_debug("Volume Up"); */
        },
        1));

    // Back Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 190, 300, 60}, "Back",
        [this]() { this->_setupMainMenu(); }, "btn_normal", "btn_hover", "btn_pressed", 1));
}

void Core::_showConnectionOverlay() {
    _menuState = MenuState::CONNECTION;
    _clearMenuUI();

    int cx = _window->GetWidth() / 2;
    int cy = _window->GetHeight() / 2;

    // Background panel
    _uiManager->addComponent(std::make_shared<UIPanel>(
        raylib::Rectangle{0, 0, (float)_window->GetWidth(), (float)_window->GetHeight()}, "menu_bg",
        0));

    // Title
    _uiManager->addComponent(
        std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy - 200, 300, 60},
                                 "CONNECT", 60, raylib::Color::RayWhite(), 1, 4.0f, "HeaderFont"));

    // Host Input
    auto hostInput =
        std::make_shared<UIInput>(raylib::Rectangle{(float)cx - 150, (float)cy - 80, 300, 50},
                                  _host, "Host...", "input_bg", 256, 1);
    _uiManager->addComponent(hostInput);

    // Port Input
    auto portInput =
        std::make_shared<UIInput>(raylib::Rectangle{(float)cx - 150, (float)cy - 10, 300, 50},
                                  std::to_string(_port), "Port...", "input_bg", 10, 1);
    _uiManager->addComponent(portInput);

    // Go Back Button
    _uiManager->addComponent(std::make_shared<UIButton>(
        raylib::Rectangle{(float)cx - 150, (float)cy + 60, 140, 50}, "Go Back",
        [this]() { this->_setupMainMenu(); }, "btn_normal", "btn_hover", "btn_pressed", 1));

    // Connect Button
    _uiManager->addComponent(std::make_shared<UIButton>(
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
        "btn_normal", "btn_hover", "btn_pressed", 1));
}

void Core::_setupGameUI() {
    // Left Scoreboard
    _uiManager->addComponent(
        std::make_shared<UIScoreboardPanel>(raylib::Rectangle{10, 50, 240, 400}, _world, 10));

    // Right Tile HUD
    _uiManager->addComponent(std::make_shared<UIHudPanel>(
        raylib::Rectangle{(float)_window->GetWidth() - 220, 50, 200, 400}, _world, _renderSystem,
        nullptr, 10));

    // Time frequency slider
    _uiManager->addComponent(
        std::make_shared<UISlider>(raylib::Rectangle{(float)_window->GetWidth() - 270,
                                                     (float)_window->GetHeight() - 60, 250, 40},
                                   _world, _network, 10));

    auto backgroundMusic = _world.spawn();
    _world.add_component(
        backgroundMusic,
        std::make_shared<ComponentMusic>(std::string("assets/sounds/music/country.mp3"), true));

    // Spawn Background parallax
    auto backgroundParallaxEntity = _world.spawn();
    _world.add_component(backgroundParallaxEntity, std::make_shared<BackgroundParallax>());

    // Spawn Moon & Sun
    auto moon = _world.spawn();
    _world.add_component(moon, std::make_shared<CelestialObject>(0.0f, 0.01f, "moon"));
    auto sun = _world.spawn();
    _world.add_component(sun, std::make_shared<CelestialObject>(3.0f, 0.0015f, "sun"));


    // Chat Panel
    float chatY = (float)_window->GetHeight() - 240.0f;
    _uiManager->addComponent(std::make_shared<UIChatPanel>(
        raylib::Rectangle{10.0f, chatY, 460.0f, 230.0f}, _chatLogs, _world, 10));
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
    _world.add_component(p1, TeamName{"Team Alpha", raylib::Color::Black()});

    auto p2 = _world.spawn();
    _world.add_component(p2, Position{5, 5});
    _world.add_component(p2, Orientation{Orientation::E});
    _world.add_component(p2, Level{4});
    _world.add_component(p2, TeamName{"Team Beta", raylib::Color::Red()});
}

} // namespace zappy
