/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIText.cpp
*/

#include "UI/UIText.hpp"
#include <raylib.h>

namespace zappy {

UIText::UIText(raylib::Rectangle bounds, const std::string& text, int fontSize, raylib::Color color,
               int zIndex)
    : AUIComponent(bounds, zIndex), _text(text), _fontSize(fontSize), _color(color) {}

void UIText::render() {
    int textW = MeasureText(_text.c_str(), _fontSize);
    int textX = (int)(_bounds.x) + ((int)_bounds.width - textW) / 2;
    int textY = (int)(_bounds.y) + ((int)_bounds.height - _fontSize) / 2;
    ::DrawText(_text.c_str(), textX, textY, _fontSize, _color);
}

} // namespace zappy
