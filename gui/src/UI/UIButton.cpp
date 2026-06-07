/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIButton.cpp
*/

#include "UI/UIButton.hpp"
#include <raylib.h>

namespace zappy {

UIButton::UIButton(raylib::Rectangle bounds, const std::string& text, std::function<void()> onClick,
                   int zIndex)
    : AUIComponent(bounds, zIndex), _text(text), _onClick(onClick), _isHovered(false),
      _isPressed(false), _normalColor(raylib::Color::LightGray()),
      _hoverColor(raylib::Color::Gray()), _pressedColor(raylib::Color::DarkGray()),
      _textColor(raylib::Color::Black()) {}

void UIButton::update(float dt, raylib::Vector2 mousePos) {
    (void)dt;
    _isHovered = CheckCollisionPointRec(mousePos, _bounds);

    if (_isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        _isPressed = true;
    }

    if (_isPressed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        _isPressed = false;
        if (_isHovered && _onClick) {
            _onClick();
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        _isPressed = false;
    }
}

void UIButton::render() {
    raylib::Color btnColor = _normalColor;
    if (_isPressed) {
        btnColor = _pressedColor;
    } else if (_isHovered) {
        btnColor = _hoverColor;
    }

    DrawRectangleRec(_bounds, btnColor);
    DrawRectangleLinesEx(_bounds, 2.0f, DARKGRAY);

    int fontSize = 20;
    int textW = MeasureText(_text.c_str(), fontSize);
    int textX = (int)(_bounds.x) + ((int)_bounds.width - textW) / 2;
    int textY = (int)(_bounds.y) + ((int)_bounds.height - fontSize) / 2;
    ::DrawText(_text.c_str(), textX, textY, fontSize, _textColor);
}

} // namespace zappy
