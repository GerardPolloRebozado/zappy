/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AError.hpp
*/

#ifndef ZAPPY_AERROR_HPP
#define ZAPPY_AERROR_HPP

#include "errors/IError.hpp"
#include <string>

namespace zappy {

class AError : public IError {
  public:
    AError(const std::string& typePrefix, const std::string& detail,
           error_severity_e severity)
        : _message(typePrefix + detail), _severity(severity) {}

    const char* what() const noexcept override { return _message.c_str(); }

    error_severity_e getSeverity() const noexcept override { return _severity; }

  protected:
    std::string _message;
    error_severity_e _severity;
};

} // namespace zappy

#endif // ZAPPY_AERROR_HPP
