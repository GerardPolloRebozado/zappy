/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** NetworkManager.hpp
*/

/**
 * @file NetworkManager.hpp
 * @brief Declaration of the NetworkManager class for Zappy protocol handling.
 */

#ifndef NETWORK_MANAGER_HPP_
#define NETWORK_MANAGER_HPP_

#include "Network/TcpSocket.hpp"
#include <functional>
#include <map>
#include <queue>
#include <string>
#include "ECS/Register.hpp"
#include "Components/ComponentTags.hpp"
namespace zappy {
/**
 * @class NetworkManager
 * @brief Manages high-level Zappy protocol interactions.
 *
 * This class uses a TcpSocket for communication and implements the Zappy
 * protocol logic, including the initial handshake and routing of server commands.
 */
class NetworkManager {
  public:
    /**
     * @brief Default constructor.
     */
    NetworkManager(Register& registry);

    /**
     * @brief Destructor. Ensures disconnection.
     */
    ~NetworkManager();

    /**
     * @brief Establishes a connection and prepares for the Zappy handshake.
     * @param host Server hostname or IP.
     * @param port Server port.
     * @return true if connection was established, false otherwise.
     */
    bool connect(const std::string& host, int port);

    /**
     * @brief Disconnects from the server and resets internal state.
     */
    void disconnect();

    /**
     * @brief Updates the network state and processes any pending protocol messages.
     *
     * This method should be called once per frame in the main application loop.
     * It performs polling, flushes outgoing data, and handles incoming commands.
     */
    void update(Register& registry);

    /**
     * @brief Sends a protocol command to the server.
     * @param cmd The command string (newline will be appended automatically).
     */
    void sendCommand(const std::string& cmd);

    /**
     * @brief Checks if the manager is currently connected to a server.
     * @return true if connected, false otherwise.
     */
    bool isConnected() const;

  /**
           * @brief Requests the logical map size from the server.
           * Sends the "msz" command. The server will respond asynchronously.
           */
  void requestMapSize();

  /**
   * @brief Requests the content of all tiles on the map from the server.
   * Sends the "mct" command. Useful for the initial world load.
   */
  void requestMapContent();

  /**
   * @brief Requests the names of all participating teams from the server.
   * Sends the "tna" command.
   */
  void requestTeamNames();

  /**
   * @brief Requests to modify the server's time unit (frequency).
   * Sends the "sst" command.
   * @param newTime The new time unit to set.
   */
  void requestTimeUpdate(int newTime);

  /**
   * @brief Requests the exact position of a specific player.
   * Sends the "ppo" command.
   * @param playerId The unique ID of the player (humanoid).
   */
  void requestPlayerPosition(int playerId);

  /**
   * @brief Requests the current level of a specific player.
   * Sends the "plv" command.
   * @param playerId The unique ID of the player (humanoid).
   */
  void requestPlayerLevel(int playerId);

  /**
   * @brief Requests the inventory content of a specific player.
   * Sends the "pin" command. Useful when the user clicks on a humanoid.
   * @param playerId The unique ID of the player (humanoid).
   */
  void requestPlayerInventory(int playerId);

  /**
   * @brief Requests the resource content of a specific tile.
   * Sends the "bct" command.
   * @param x Horizontal coordinate of the tile.
   * @param y Vertical coordinate of the tile.
   */
  void requestTileContent(int x, int y);

  private:
    TcpSocket _socket;           /**< The underlying TCP socket. */
    bool _isHandshakeDone;       /**< Flag indicating if the Zappy handshake is complete. */
    Register& _registry;         /**< The ECS registry for managing game entities and components. */

    /**
     * @brief Routes a received protocol message to the appropriate handler.
     * @param message The raw message string from the server.
     */
    void _handleProtocolMessage(const std::string& message, Register& registry);

    /**
     * @brief Processes the initial "WELCOME" handshake.
     * @param message The message received during the handshake phase.
     */
    void _processHandshake(const std::string& message);

    /** @name Protocol Handlers */
    /** @{ */
    /** @brief Handles new player connection (pnw). */
    void _handlePlayerConnection(const std::string& args, Register& registry);
    /** @} */
};
} // namespace zappy

#endif /* !NETWORK_MANAGER_HPP_ */
