/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIButton.cpp
*/

#include "UI/UIButton.hpp"

namespace zappy {

UIButton::UIButton(raylib::Rectangle bounds, const std::string& text, std::function<void()> onClick,
                   int zIndex)
    : AUIComponent(bounds, zIndex), _onClick(onClick), _isHovered(false), _isPressed(false),
      _normalColor(raylib::Color::LightGray()), _hoverColor(raylib::Color::Gray()),
      _pressedColor(raylib::Color::DarkGray()) {
    _label = std::make_unique<UIText>(bounds, text, 20, raylib::Color::Black(), zIndex, 1.5f);
}

void UIButton::update(float dt, raylib::Vector2 mousePos,
                      std::shared_ptr<std::vector<UIEvent>> events) {
    (void)dt;
    _isHovered = _bounds.CheckCollision(mousePos);

    for (auto it = events->begin(); it != events->end();) {
        bool consumed = false;

        if (it->type == UIEventType::MOUSE_PRESSED_LEFT && _isHovered) {
            _isPressed = true;
            consumed = true;
        }

        if (it->type == UIEventType::MOUSE_RELEASED_LEFT) {
            if (_isPressed) {
                if (_isHovered && _onClick) {
                    _onClick();
                }
                _isPressed = false;
                consumed = true;
            }
        }

        if (consumed) {
            it = events->erase(it);
        } else {
            ++it;
        }
    }
}

void UIButton::render() {
    raylib::Color btnColor = _normalColor;
    if (_isPressed) {
        btnColor = _pressedColor;
    } else if (_isHovered) {
        btnColor = _hoverColor;
    }

    _bounds.Draw(btnColor);
    _bounds.DrawLines(DARKGRAY, 2.0f);

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
