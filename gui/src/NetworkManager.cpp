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

NetworkManager::NetworkManager(Register& registry, RenderSystem& renderSystem)
    : _registry(registry), _renderSystem(renderSystem), _isHandshakeDone(false) {}

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

void NetworkManager::_handleProtocolMessage(const std::string& message) {
    std::istringstream iss(message);
    std::string cmd;
    if (!(iss >> cmd)) {
        return;
    }

    if (cmd == "msz") {
        _handleMapSize(message.substr(message.find(' ') + 1));
    } else if (cmd == "bct") {
        _handleTileContent(message.substr(message.find(' ') + 1));
    } else if (cmd == "tna") {
        _handleTeamNames(message.substr(message.find(' ') + 1));
    } else if (cmd == "pnw") {
        _handlePlayerConnection(message.substr(message.find(' ') + 1));
    }
}

void NetworkManager::_handleMapSize(const std::string& args) {
    std::cout << "Protocol: Map size update [" << args << "]" << std::endl;
    std::istringstream iss(args);
    int width, height;
    if (!(iss >> width >> height)) {
        return;
    }
    _renderSystem.centerCamera(width, height);
}

void NetworkManager::_handleTileContent(const std::string& args) {
    std::istringstream iss(args);
    int x, y, q0, q1, q2, q3, q4, q5, q6, t_type;
    if (!(iss >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6 >> t_type)) {
        return;
    }

    int tileEntity = -1;
    for (auto const& [ent, pos] : _registry._positions) {
        if (pos.x == x && pos.y == y && _registry._terrainTypes.count(ent)) {
            tileEntity = ent;
            break;
        }
    }

    if (tileEntity == -1) {
        tileEntity = _registry.createEntity();
        _registry._positions[tileEntity] = {x, y};
    }

    _registry._inventories[tileEntity] = {q0, q1, q2, q3, q4, q5, q6};
    _registry._terrainTypes[tileEntity] = {(zappy::TerrainType::Type)t_type};

    if (x == 0 && y == 0) {
        std::cout << "Debug: Received tile(0,0) terrain type: " << t_type << std::endl;
    }
}

void NetworkManager::_handleTeamNames(const std::string& args) {
    std::cout << "Protocol: Team name: " << args << std::endl;
}

void NetworkManager::_handlePlayerConnection(const std::string& args) {
    // Parse #n X Y O L N
}

} // namespace zappy
