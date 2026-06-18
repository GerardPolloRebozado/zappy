/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** NetworkManager.cpp
*/

#include "NetworkManager.hpp"
#include "Commands/FactoryCommands.hpp"
#include "Logging/Logger.hpp"
#include "errors/IError.hpp"
#include <sstream>

namespace zappy {

NetworkManager::~NetworkManager() { disconnect(); }

bool NetworkManager::connect(const std::string& host, int port) {
    _isHandshakeDone = false;
    return _socket.connect(host, port);
}

void NetworkManager::disconnect() {
    _socket.disconnect();
    _isHandshakeDone = false;
}

void NetworkManager::update(World& world) {
    if (!_socket.isConnected()) {
        return;
    }

    // Use poll to check for activity without blocking the GUI unnecessarily.
    // Even with blocking sockets, a 0 timeout poll allows us to skip read/send if not ready.
    if (_socket.pollSocket(0)) {
        if (_socket.canRead()) {
            _socket.receive();

            std::string line;
            while (!(line = _socket.readLine()).empty()) {
                if (!_isHandshakeDone) {
                    _processHandshake(line);
                } else {
                    _handleProtocolMessage(line, world);
                }
            }
        }
        if (_socket.canWrite()) {
            _socket.flush();
        }
    }
}

void NetworkManager::sendCommand(const std::string& cmd) {
    log_debug("Sending new command: " + cmd);
    _socket.send(cmd + "\n");
}

bool NetworkManager::isConnected() const { return _socket.isConnected(); }

void NetworkManager::requestMapSize() { sendCommand("msz"); }
void NetworkManager::requestMapContent() { sendCommand("mct"); }
void NetworkManager::requestTeamNames() { sendCommand("tna"); }
void NetworkManager::requestTimeUpdate(int newTime) {
    sendCommand("sst " + std::to_string(newTime));
}
void NetworkManager::requestPlayerPosition(int playerId) {
    sendCommand("ppo #" + std::to_string(playerId));
}
void NetworkManager::requestPlayerLevel(int playerId) {
    sendCommand("plv #" + std::to_string(playerId));
}
void NetworkManager::requestPlayerInventory(int playerId) {
    sendCommand("pin #" + std::to_string(playerId));
}
void NetworkManager::requestTileContent(int x, int y) {
    sendCommand("bct " + std::to_string(x) + " " + std::to_string(y));
}

void NetworkManager::_processHandshake(const std::string& message) {
    if (message == "WELCOME") {
        log_debug("Received WELCOME, sending GRAPHIC");
        sendCommand("GRAPHIC");
        _isHandshakeDone = true;

        // Immediately request basic world info
        sendCommand("msz");
        sendCommand("sgt");
        sendCommand("mct");
        sendCommand("tna");
    }
}

// should return info somewhere?
void NetworkManager::_handleProtocolMessage(const std::string& message, World& world) {
    log_info("Received response: " + message);
    std::istringstream iss(message);
    std::string cmd;
    if (!(iss >> cmd)) {
        return;
    }

    try {
        ACommand& command = FactoryCommands::getCommand(cmd);
        std::string args = message.substr(cmd.length() + 1);
        command.execute(args, world);
    } catch (const IError& e) {
        log_error(e.what());
    }
}

} // namespace zappy
