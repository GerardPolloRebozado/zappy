/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIButton.cpp
*/

#include "UI/UIButton.hpp"
#include "UI/AUIComponent.hpp"

namespace zappy {

UIButton::UIButton(raylib::Rectangle bounds, const std::string& text, std::function<void()> onClick,
                   int zIndex)
    : AUIComponent(bounds, onClick, zIndex), _normalColor(raylib::Color(0, 80, 200, 255)),
      _hoverColor(raylib::Color(0, 100, 255, 255)), _pressedColor(raylib::Color(0, 50, 150, 255)) {
    _label = std::make_unique<UIText>(bounds, text, 20, raylib::Color::RayWhite(), zIndex, 1.5f);
}

void UIButton::update(float dt, raylib::Vector2 mousePos,
                      std::shared_ptr<std::vector<UIEvent>> events) {
    AUIComponent::update(dt, mousePos, events);
}

void UIButton::render() {
    raylib::Color btnColor = _normalColor;
    if (_isPressed) {
        btnColor = _pressedColor;
    } else if (_isHovered) {
        btnColor = _hoverColor;
    }

    _bounds.Draw(btnColor);
    _bounds.DrawLines(raylib::Color(200, 200, 255, 100), 2.0f);

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
