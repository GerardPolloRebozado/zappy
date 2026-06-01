/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** TcpSocket.hpp
*/

/**
 * @file TcpSocket.hpp
 * @brief Declaration of the TcpSocket class for low-level TCP communication.
 */

#ifndef TCP_SOCKET_HPP_
#define TCP_SOCKET_HPP_

#include <poll.h>
#include <string>
#include <vector>

namespace zappy {
/**
 * @class TcpSocket
 * @brief Encapsulates a TCP socket with buffered I/O and polling support.
 *
 * This class provides a clean interface for TCP communication, using poll()
 * for multiplexing and maintaining internal buffers for reading and writing.
 */
class TcpSocket {
  public:
    /**
     * @brief Default constructor. Initializes the socket in a disconnected state.
     */
    TcpSocket();

    /**
     * @brief Destructor. Automatically closes the socket if connected.
     */
    ~TcpSocket();

    /**
     * @brief Connects to a remote host.
     * @param host The hostname or IP address of the server.
     * @param port The port number to connect to.
     * @return true if the connection was successful, false otherwise.
     */
    bool connect(const std::string& host, int port);

    /**
     * @brief Disconnects from the server and clears all buffers.
     */
    void disconnect();

    /**
     * @brief Polls the socket for events (read/write readiness).
     * @param timeout Timeout in milliseconds for the poll call.
     * @return true if there are events to handle, false otherwise.
     */
    bool pollSocket(int timeout = 0);

    /**
     * @brief Flushes the write buffer to the socket.
     *
     * Sends as much data as possible from the internal write buffer to the OS socket.
     * Only performs the write if the socket is ready for output.
     */
    void flush();

    /**
     * @brief Receives data from the socket into the internal read buffer.
     *
     * Reads available data from the OS socket and appends it to the internal read buffer.
     * Only performs the read if the socket is ready for input.
     */
    void receive();

    /**
     * @brief Queues data to be sent.
     * @param data The string to append to the write buffer.
     */
    void send(const std::string& data);

    /**
     * @brief Reads a complete line from the read buffer.
     *
     * Extracts data up to the first newline character ('\n') from the read buffer.
     * Handles both LF and CRLF line endings.
     *
     * @return The extracted line (without the newline), or an empty string if no complete line is
     * available.
     */
    std::string readLine();

    /**
     * @brief Checks if the socket is currently connected.
     * @return true if connected, false otherwise.
     */
    bool isConnected() const { return _fd != -1; }

    /**
     * @brief Checks if the socket is ready for reading (populated after pollSocket).
     * @return true if ready, false otherwise.
     */
    bool canRead() const { return _revents & POLLIN; }

    /**
     * @brief Checks if the socket is ready for writing (populated after pollSocket).
     * @return true if ready, false otherwise.
     */
    bool canWrite() const { return _revents & POLLOUT; }

  private:
    int _fd;                  /**< The raw socket file descriptor. */
    short _revents;           /**< The events returned by the last poll call. */
    std::string _readBuffer;  /**< Internal buffer for incoming data. */
    std::string _writeBuffer; /**< Internal buffer for outgoing data. */
};
} // namespace zappy

#endif /* !TCP_SOCKET_HPP_ */
