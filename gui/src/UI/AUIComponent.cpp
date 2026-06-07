/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AUIComponent.cpp
*/

#include "UI/AUIComponent.hpp"

namespace zappy {

AUIComponent::AUIComponent(raylib::Rectangle bounds, int zIndex)
    : _bounds(bounds), _zIndex(zIndex), _isVisible(true) {}

void AUIComponent::update(float dt, raylib::Vector2 mousePos,
                          std::shared_ptr<std::vector<UIEvent>> events) {
    (void)dt;
    (void)mousePos;
    (void)events;
}

int AUIComponent::getZIndex() const { return _zIndex; }

void AUIComponent::setZIndex(int zIndex) { _zIndex = zIndex; }

raylib::Rectangle AUIComponent::getBounds() const { return _bounds; }

void AUIComponent::setBounds(raylib::Rectangle bounds) { _bounds = bounds; }

bool AUIComponent::isVisible() const { return _isVisible; }

void AUIComponent::setVisible(bool visible) { _isVisible = visible; }

} // namespace zappy
