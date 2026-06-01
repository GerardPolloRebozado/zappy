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
    NetworkManager();

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
    void update();

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

  private:
    TcpSocket _socket;     /**< The underlying TCP socket. */
    bool _isHandshakeDone; /**< Flag indicating if the Zappy handshake is complete. */

    /**
     * @brief Routes a received protocol message to the appropriate handler.
     * @param message The raw message string from the server.
     */
    void _handleProtocolMessage(const std::string& message);

    /**
     * @brief Processes the initial "WELCOME" handshake.
     * @param message The message received during the handshake phase.
     */
    void _processHandshake(const std::string& message);

    /** @name Protocol Handlers */
    /** @{ */
    void _handleMapSize(const std::string& args);
    void _handleTileContent(const std::string& args);
    void _handleTeamNames(const std::string& args);
    void _handlePlayerConnection(const std::string& args);
    /** @} */
};
} // namespace zappy

#endif /* !NETWORK_MANAGER_HPP_ */
