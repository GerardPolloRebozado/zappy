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
    : AUIComponent(bounds, zIndex), _onClick(onClick), _isHovered(false), _isPressed(false),
      _normalColor(raylib::Color::LightGray()), _hoverColor(raylib::Color::Gray()),
      _pressedColor(raylib::Color::DarkGray()) {
    _label = std::make_unique<UIText>(bounds, text, 20, raylib::Color::Black(), zIndex);
}

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

    if (_label) {
        _label->render();
    }
}

void UIButton::setBounds(raylib::Rectangle bounds) {
    _bounds = bounds;
    if (_label) {
        _label->setBounds(bounds);
    }
}

void UIButton::setZIndex(int zIndex) {
    _zIndex = zIndex;
    if (_label) {
        _label->setZIndex(zIndex);
    }
}

} // namespace zappy
