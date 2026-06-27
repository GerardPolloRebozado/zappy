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

void Core::_update() {

    float dt = _window->GetFrameTime();
    _uiManager->update(dt);

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
            _world.despawn_all_entities();
            _setupMainMenu();
            return;
        }

        _network.update(_world);
        if (_network.isConnected()) {
            _simulationSystem.update(_world);
            _animationSystem.update(_world);
            _renderSystem.update(_world, dt);
            _particleSystem.update(_world, dt);
            _musicSystem.update(_world, dt);
            _backgroundSystem.update(_world, dt);
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

    _uiManager->render();

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
        _uiManager->addComponent(
            std::make_shared<UIText>(raylib::Rectangle{(float)cx - 150, (float)cy + 110, 300, 30},
                                     "Connection Failed!", 20, raylib::Color::Red(), 12, 2.0f));
    }
}

} // namespace zappy
