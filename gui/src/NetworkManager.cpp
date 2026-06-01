/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** NetworkManager.cpp
*/

#include "NetworkManager.hpp"
#include <iostream>
#include <sstream>

namespace zappy {

NetworkManager::NetworkManager() : _isHandshakeDone(false) {}

NetworkManager::~NetworkManager() { disconnect(); }

bool NetworkManager::connect(const std::string& host, int port) {
    _isHandshakeDone = false;
    return _socket.connect(host, port);
}

void NetworkManager::disconnect() {
    _socket.disconnect();
    _isHandshakeDone = false;
}

void NetworkManager::update() {
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
                    _handleProtocolMessage(line);
                }
            }
        }
        if (_socket.canWrite()) {
            _socket.flush();
        }
    }
}

void NetworkManager::sendCommand(const std::string& cmd) { _socket.send(cmd + "\n"); }

bool NetworkManager::isConnected() const { return _socket.isConnected(); }

void NetworkManager::_processHandshake(const std::string& message) {
    if (message == "WELCOME") {
        std::cout << "Received WELCOME, sending GRAPHIC" << std::endl;
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
void NetworkManager::_handleProtocolMessage(const std::string& message) {
    std::istringstream iss(message);
    std::string cmd;
    iss >> cmd;

    if (cmd == "msz") {
        _handleMapSize(message.substr(3));
    } else if (cmd == "bct") {
        _handleTileContent(message.substr(3));
    } else if (cmd == "tna") {
        _handleTeamNames(message.substr(3));
    } else if (cmd == "pnw") {
        _handlePlayerConnection(message.substr(3));
    } else {
        // std::cout << "Unprocessed server command: " << cmd << std::endl;
    }
}

void NetworkManager::_handleMapSize(const std::string& args) {
    std::cout << "Protocol: Map size update [" << args << "]" << std::endl;
    // Parse X and Y, update world state
}

void NetworkManager::_handleTileContent(const std::string& args) {
    // Parse X Y q0 q1...
}

void NetworkManager::_handleTeamNames(const std::string& args) {
    std::cout << "Protocol: Team name: " << args << std::endl;
}

void NetworkManager::_handlePlayerConnection(const std::string& args) {
    // Parse #n X Y O L N
}

} // namespace zappy
