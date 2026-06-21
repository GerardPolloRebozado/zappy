/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** Core.cpp
*/

#include "Core.hpp"
#include "Color.hpp"
#include "Commands/FactoryCommands.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
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

#include "UI/UIRadio.hpp"

namespace zappy {

Core::Core(int port, const std::string& host) : _port(port), _host(host) {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    _window = std::make_unique<raylib::Window>(1280, 720, "Zappy");
    _window->SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    // Prevent ESC from instantly closing the entire application
    _window->SetExitKey(0);

    _window->Maximize();

    // Initialize the speakers
    InitAudioDevice();

    // Load assets and hide default cursor so our custom cursor renders immediately
    AssetManager::getInstance().loadAll();
    raylib::Window::HideCursor();

    _chatLogs = std::make_shared<ChatLogs>();
    FactoryCommands::setChatLogs(_chatLogs);
    _uiManager = std::make_shared<UIManager>();
    _appState = AppState::MENU;
    _setupMainMenu();
}

Core::~Core() {
    AssetManager::getInstance().unloadAll();
    CloseAudioDevice();
}

void Core::run() {
    while (!_window->ShouldClose() && !_shouldClose) {
        _update();
        _render();
    }
}

} // namespace zappy
