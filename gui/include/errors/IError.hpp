/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** IError.hpp
*/

#ifndef ZAPPY_IERROR_HPP
#define ZAPPY_IERROR_HPP

namespace zappy {

enum class error_severity_e {
    LOW,
    MODERATE,
    CRITICAL,
    FATAL
};

class IError {
  public:
    virtual ~IError() = default;

    virtual const char* what() const noexcept = 0;
    virtual error_severity_e getSeverity() const noexcept = 0;
};

} // namespace zappy

#endif // ZAPPY_IERROR_HPP
