/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** CoreErrors.hpp
*/

#ifndef ZAPPY_COREERRORS_HPP
#define ZAPPY_COREERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

class ErrorConfig : public AError {
  public:
    explicit ErrorConfig(const std::string& detail,
                         error_severity_e severity = error_severity_e::FATAL)
        : AError("Type: Config\nMessage: ", detail, severity) {}
};

inline int parsePort(const std::string& portStr) {
    int port = 0;

    try {
        port = std::stoi(portStr);
    } catch (...) {
        throw ErrorConfig("Invalid port number.");
    }

    if (port == 0) {
        throw ErrorConfig("Invalid port number.");
    }

    return port;
}

} // namespace zappy

#endif // ZAPPY_COREERRORS_HPP
