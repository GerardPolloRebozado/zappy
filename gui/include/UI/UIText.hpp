/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIText.hpp
*/

#ifndef ZAPPY_UITEXT_HPP
#define ZAPPY_UITEXT_HPP

#include "AUIComponent.hpp"
#include <string>

namespace zappy {

class UIText : public AUIComponent {
  public:
    UIText(raylib::Rectangle bounds, const std::string& text, int fontSize, raylib::Color color,
           int zIndex = 0);
    ~UIText() override = default;

    void render() override;

  private:
    std::string _text;
    int _fontSize;
    raylib::Color _color;
};

} // namespace zappy

#endif // ZAPPY_UITEXT_HPP
