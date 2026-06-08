/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** CommandsErrors.hpp
*/

#ifndef ZAPPY_COMMANDSERRORS_HPP
#define ZAPPY_COMMANDSERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

class ErrorProtocol : public AError {
  public:
    explicit ErrorProtocol(const std::string& detail,
                           error_severity_e severity = error_severity_e::LOW)
        : AError("Type: Protocol\nMessage: ", detail, severity) {}
};

} // namespace zappy

#endif // ZAPPY_COMMANDSERRORS_HPP
