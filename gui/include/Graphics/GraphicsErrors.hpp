/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** GraphicsErrors.hpp
*/

#ifndef ZAPPY_GRAPHICSERRORS_HPP
#define ZAPPY_GRAPHICSERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

class ErrorAsset : public AError {
  public:
    explicit ErrorAsset(const std::string& detail,
                        error_severity_e severity = error_severity_e::CRITICAL)
        : AError("Type: Asset\nMessage: ", detail, severity) {}
};

} // namespace zappy

#endif // ZAPPY_GRAPHICSERRORS_HPP
