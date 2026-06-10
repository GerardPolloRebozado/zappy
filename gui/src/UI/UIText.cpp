/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIText.cpp
*/

#include "UI/UIText.hpp"
#include "Graphics/AssetManager.hpp"

namespace zappy {

UIText::UIText(raylib::Rectangle bounds, const std::string& text, int fontSize, raylib::Color color,
               int zIndex, float spacing)
    : AUIComponent(bounds, zIndex), _text(text), _fontSize(fontSize), _color(color),
      _spacing(spacing) {}

void UIText::render() {
    raylib::Text label(_text, (float)_fontSize, _color,
                       AssetManager::getInstance().getFont("BoldPixels"), _spacing);
    raylib::Vector2 textSize = label.MeasureEx();

    int textX = (int)(_bounds.x) + ((int)_bounds.width - (int)textSize.x) / 2;
    int textY = (int)(_bounds.y) + ((int)_bounds.height - (int)textSize.y) / 2;

    label.Draw(raylib::Vector2((float)textX, (float)textY));
}

void UIText::setSpacing(float spacing) { _spacing = spacing; }

float UIText::getSpacing() const { return _spacing; }

} // namespace zappy
