/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** NetworkErrors.hpp
*/

/**
 * @file NetworkErrors.hpp
 * @brief Network-layer exceptions for TCP and protocol I/O failures.
 */

#ifndef ZAPPY_NETWORKERRORS_HPP
#define ZAPPY_NETWORKERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

/**
 * @class ErrorNetwork
 * @brief Exception thrown when a network operation fails (connect, send, receive).
 */
class ErrorNetwork : public AError {
  public:
    explicit ErrorNetwork(const std::string& detail,
                          error_severity_e severity = error_severity_e::CRITICAL)
        : AError("Type: Network\nMessage: ", detail, severity) {}
};

} // namespace zappy

#endif // ZAPPY_NETWORKERRORS_HPP
