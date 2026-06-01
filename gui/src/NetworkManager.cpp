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

void NetworkManager::update(Register& registry) {
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
                    _handleProtocolMessage(line, registry);
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

void NetworkManager::requestMapSize() { sendCommand("msz"); }
void NetworkManager::requestMapContent() { sendCommand("mct"); }
void NetworkManager::requestTeamNames() { sendCommand("tna"); }
void NetworkManager::requestTimeUpdate(int newTime) { sendCommand("sst " + std::to_string(newTime)); }
void NetworkManager::requestPlayerPosition(int playerId) { sendCommand("ppo #" + std::to_string(playerId)); }
void NetworkManager::requestPlayerLevel(int playerId) { sendCommand("plv #" + std::to_string(playerId)); }
void NetworkManager::requestPlayerInventory(int playerId) { sendCommand("pin #" + std::to_string(playerId)); }
void NetworkManager::requestTileContent(int x, int y) { sendCommand("bct " + std::to_string(x) + " " + std::to_string(y)); }

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
void NetworkManager::_handleProtocolMessage(const std::string& message, Register& registry) {
    std::istringstream iss(message);
    std::string cmd;
    iss >> cmd;

    if (cmd == "msz") {
        _handleMapSize(message.substr(4), registry);
    } else if (cmd == "bct") {
        _handleTileContent(message.substr(4), registry);
    } else if (cmd == "tna") {
        _handleTeamNames(message.substr(4), registry);
    } else if (cmd == "pnw") {
        _handlePlayerConnection(message.substr(4), registry);
    } else {
        // std::cout << "Unprocessed server command: " << cmd << std::endl;
    }
}

void NetworkManager::_handleMapSize(const std::string& args, Register& registry) {
    std::istringstream iss(args);
    int width, height;

    if (iss >> width >> height) {
        std::cout << "Protocol: Map size update " << width << "x" << height << std::endl;
    }
}

void NetworkManager::_handleTileContent(const std::string& args, Register& registry) {
    std::istringstream iss(args);
    int x, y, q0, q1, q2, q3, q4, q5, q6;

    if (iss >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6) {
        int tileEntity = registry.createEntity();
        registry._tileTags[tileEntity] = TileTag{};
        registry._positions[tileEntity] = Position{x, y};
        registry._inventories[tileEntity] = Inventory{q0, q1, q2, q3, q4, q5, q6};
        std::cout << "Tile created at (" << x << "," << y << ") with " << q0 << " food." << std::endl;
    }
}

void NetworkManager::_handleTeamNames(const std::string& args, Register& registry) {
    std::cout << "Protocol: Team name: " << args << std::endl;
}

void NetworkManager::_handlePlayerConnection(const std::string& args, Register& registry) {
    // Parse #n X Y O L N
}

} // namespace zappy