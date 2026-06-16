/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** IError.hpp
*/

/**
 * @file IError.hpp
 * @brief Interface for typed exceptions used across the GUI.
 */

#ifndef ZAPPY_IERROR_HPP
#define ZAPPY_IERROR_HPP

namespace zappy {

/**
 * @enum error_severity_e
 * @brief Severity level attached to a GUI error.
 */
enum class error_severity_e { LOW, MODERATE, CRITICAL, FATAL };

/**
 * @class IError
 * @brief Abstract base interface for all GUI exceptions.
 *
 * Mirrors std::exception with an additional severity hint so callers can
 * distinguish recoverable protocol issues from fatal configuration errors.
 */
class IError {
  public:
    virtual ~IError() = default;

    virtual const char* what() const noexcept = 0;
    virtual error_severity_e getSeverity() const noexcept = 0;
};

} // namespace zappy

#endif // ZAPPY_IERROR_HPP
