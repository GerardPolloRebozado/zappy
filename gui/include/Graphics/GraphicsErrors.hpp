/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** GraphicsErrors.hpp
*/

/**
 * @file GraphicsErrors.hpp
 * @brief Asset and graphics-related exceptions.
 */

#ifndef ZAPPY_GRAPHICSERRORS_HPP
#define ZAPPY_GRAPHICSERRORS_HPP

#include "errors/AError.hpp"
#include <string>

namespace zappy {

/**
 * @class ErrorAsset
 * @brief Exception thrown when a model, texture or shader cannot be loaded or found.
 */
class ErrorAsset : public AError {
  public:
    explicit ErrorAsset(const std::string& detail,
                        error_severity_e severity = error_severity_e::CRITICAL)
        : AError("Type: Asset\nMessage: ", detail, severity) {}
};

} // namespace zappy

#endif // ZAPPY_GRAPHICSERRORS_HPP
