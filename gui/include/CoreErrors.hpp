/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** CoreErrors.hpp
*/

/**
 * @file CoreErrors.hpp
 * @brief Configuration-related errors and helpers for the GUI entry point.
 */

#ifndef ZAPPY_COREERRORS_HPP
#define ZAPPY_COREERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

/**
 * @class ErrorConfig
 * @brief Exception thrown when CLI arguments or core configuration are invalid.
 */
class ErrorConfig : public AError {
  public:
    explicit ErrorConfig(const std::string& detail,
                         error_severity_e severity = error_severity_e::FATAL)
        : AError("Type: Config\nMessage: ", detail, severity) {}
};

/**
 * @brief Parses and validates a port string from the command line.
 * @param portStr Port argument passed to the GUI (e.g. from @c -p).
 * @return Parsed port number.
 * @throws ErrorConfig If the string is not a valid non-zero port.
 */
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
