/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AError.hpp
*/

/**
 * @file AError.hpp
 * @brief Abstract implementation of @ref zappy::IError with formatted messages.
 */

#ifndef ZAPPY_AERROR_HPP
#define ZAPPY_AERROR_HPP

#include "errors/IError.hpp"
#include <string>

namespace zappy {

/**
 * @class AError
 * @brief Base class for concrete GUI errors.
 *
 * Stores a formatted message and severity. Domain-specific error types
 * (network, protocol, assets, etc.) inherit from this class and pass a
 * type prefix and detail string to the constructor.
 */
class AError : public IError {
  public:
    AError(const std::string& typePrefix, const std::string& detail, error_severity_e severity)
        : _message(typePrefix + detail), _severity(severity) {}

    const char* what() const noexcept override { return _message.c_str(); }

    error_severity_e getSeverity() const noexcept override { return _severity; }

  protected:
    std::string _message;
    error_severity_e _severity;
};

} // namespace zappy

#endif // ZAPPY_AERROR_HPP
