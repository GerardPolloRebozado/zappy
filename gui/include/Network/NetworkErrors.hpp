/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** NetworkErrors.hpp
*/

#ifndef ZAPPY_NETWORKERRORS_HPP
#define ZAPPY_NETWORKERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

class ErrorNetwork : public AError {
  public:
    explicit ErrorNetwork(const std::string& detail,
                          error_severity_e severity = error_severity_e::CRITICAL)
        : AError("Type: Network\nMessage: ", detail, severity) {}
};

} // namespace zappy

#endif // ZAPPY_NETWORKERRORS_HPP
