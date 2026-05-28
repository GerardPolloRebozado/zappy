/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** TcpSocket.cpp
*/

#include "Network/TcpSocket.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace zappy {

TcpSocket::TcpSocket() : _fd(-1), _revents(0) {}

TcpSocket::~TcpSocket() { disconnect(); }

bool TcpSocket::connect(const std::string& host, int port) {
    struct hostent* server;
    struct sockaddr_in serv_addr;

    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) {
        return false;
    }

    server = gethostbyname(host.c_str());
    if (server == nullptr) {
        close(_fd);
        _fd = -1;
        return false;
    }

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (::connect(_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(_fd);
        _fd = -1;
        return false;
    }

    return true;
}

void TcpSocket::disconnect() {
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
    _readBuffer.clear();
    _writeBuffer.clear();
    _revents = 0;
}

bool TcpSocket::pollSocket(int timeout) {
    if (_fd == -1) {
        return false;
    }

    struct pollfd pfd;
    pfd.fd = _fd;
    pfd.events = POLLIN;
    if (!_writeBuffer.empty()) {
        pfd.events |= POLLOUT;
    }

    int ret = poll(&pfd, 1, timeout);
    if (ret > 0) {
        _revents = pfd.revents;
        return true;
    }
    _revents = 0;
    return false;
}

void TcpSocket::flush() {
    if (_fd == -1 || _writeBuffer.empty() || !(_revents & POLLOUT)) {
        return;
    }

    // Since the socket is blocking, we only send if poll said we can
    // We send the whole buffer or as much as the OS allows
    ssize_t bytesWritten = ::send(_fd, _writeBuffer.c_str(), _writeBuffer.length(), 0);
    if (bytesWritten > 0) {
        _writeBuffer.erase(0, bytesWritten);
    } else if (bytesWritten < 0) {
        disconnect();
    }
}

void TcpSocket::receive() {
    if (_fd == -1 || !(_revents & POLLIN)) {
        return;
    }

    char buffer[4096];
    ssize_t bytesRead = ::read(_fd, buffer, sizeof(buffer) - 1);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        _readBuffer += buffer;
    } else {
        disconnect();
    }
}

void TcpSocket::send(const std::string& data) { _writeBuffer += data; }

std::string TcpSocket::readLine() {
    size_t pos = _readBuffer.find('\n');
    if (pos == std::string::npos) {
        return "";
    }

    std::string line = _readBuffer.substr(0, pos);
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    _readBuffer.erase(0, pos + 1);
    return line;
}

} // namespace zappy
