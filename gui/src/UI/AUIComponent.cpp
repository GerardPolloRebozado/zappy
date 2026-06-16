/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AUIComponent.cpp
*/

#include "UI/AUIComponent.hpp"
namespace zappy {

AUIComponent::AUIComponent(raylib::Rectangle bounds, std::function<void()> onClick, int zIndex)
    : _bounds(bounds), _zIndex(zIndex), _isVisible(true), _onClick(onClick), _isHovered(false),
      _isPressed(false) {}

void AUIComponent::update(float dt, raylib::Vector2 mousePos,
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
                if (_isHovered && _onClick != nullptr) {
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

int AUIComponent::getZIndex() const { return _zIndex; }

void AUIComponent::setZIndex(int zIndex) { _zIndex = zIndex; }

raylib::Rectangle AUIComponent::getBounds() const { return _bounds; }

void AUIComponent::setBounds(raylib::Rectangle bounds) { _bounds = bounds; }

bool AUIComponent::isVisible() const { return _isVisible; }

void AUIComponent::setVisible(bool visible) { _isVisible = visible; }

} // namespace zappy
