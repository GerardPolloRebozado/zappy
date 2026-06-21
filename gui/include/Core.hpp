/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** Core.hpp
*/

#ifndef ZAPPY_CORE_HPP
#define ZAPPY_CORE_HPP

#include "ECS/World.hpp"
#include "Logging/ChatLogs.hpp"
#include "NetworkManager.hpp"
#include "Systems/AnimationSystem.hpp"
#include "Systems/MusicSystem.hpp"
#include "Systems/ParticleSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "UI/UIManager.hpp"
#include <filesystem>
#include <fstream>
#include <memory>
#include <raylib-cpp.hpp>
#include <string>

#define LOG_LEVEL 1 2 3
#define ERROR 1
#define WARNING 2
#define INFO 3

namespace zappy {

enum class AppState { MENU, PLAYING };
enum class MenuState { MAIN, SETTINGS, CONNECTION };

/**
 * @class Core
 * @brief Main orchestration class for the Zappy GUI.
 *
 * Encapsulates the application lifecycle, including initialization,
 * the main game loop, and the coordination between network and rendering systems.
 */
class Core {
  public:
    /**
     * @brief Construct a new Core object.
     * @param port The server port to connect to.
     * @param host The server hostname or IP.
     */
    Core(int port, const std::string& host);

    /**
     * @brief Destroy the Core object.
     */
    ~Core();

    /**
     * @brief Starts the application and enters the main loop.
     */
    void run();

  private:
    /**
     * @brief Performs a single update of the application state.
     */
    void _update();

    /**
     * @brief Renders the current state of the application.
     */
    void _render();

    /**
     * @brief Populates the world with dummy data for testing/demo purposes.
     * @todo Remove once server integration is complete.
     */
    void _setupTestingData();

    /**
     * @brief Builds the main menu UI components.
     */
    void _setupMainMenu();

    /**
     * @brief Clears the current UI and builds the connection screen.
     */
    void _showConnectionOverlay();

    /**
     * @brief Clears the current UI and builds the settings menu.
     */
    void _setupSettingsMenu();

    /**
     * @brief Adds in-game HUD components (scoreboard and tile inspector).
     */
    void _setupGameUI();

    /**
     * @brief Clears all UI components from the manager.
     */
    void _clearMenuUI();

    /**
     * @brief Attempts a TCP connection to the server and transitions to the PLAYING state.
     * @param host The server hostname or IP address.
     * @param port The server port.
     */
    void _connectToServer(const std::string& host, int port);

    std::unique_ptr<raylib::Window> _window; ///< The Raylib window instance.
    World _world;                            ///< The ECS world instance.
    NetworkManager _network;                 ///< Handles server communication.
    RenderSystem _renderSystem;              ///< Handles world and UI rendering.
    std::shared_ptr<UIManager> _uiManager;   ///< Handles OOP UI.
    ParticleSystem _particleSystem;          ///< Handles particle system updates.
    AnimationSystem _animationSystem;        ///< Handles entity animations.
    MusicSystem _musicSystem;                ///< Handles music system updates.
    int _port;                               ///< Server port.
    std::string _host;                       ///< Server host.
    AppState _appState = AppState::MENU;     ///< Current application state.
    MenuState _menuState = MenuState::MAIN;  ///< Current menu state.
    bool _shouldClose = false;               ///< Flag to request application shutdown.
    std::shared_ptr<ChatLogs> _chatLogs;     ///< Handle the logs of the chat
    std::vector<std::string> _bckMusic;      ///< List of Bck Music
};

} // namespace zappy

#endif // ZAPPY_CORE_HPP
