/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UtilsErrors.hpp
*/

#ifndef ZAPPY_UTILSERRORS_HPP
#define ZAPPY_UTILSERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

class ErrorConfig : public AError {
  public:
    explicit ErrorConfig(const std::string& detail,
                         error_severity_e severity = error_severity_e::FATAL)
        : AError("Type: Config\nMessage: ", detail, severity) {}
};

} // namespace zappy

#endif // ZAPPY_UTILSERRORS_HPP
