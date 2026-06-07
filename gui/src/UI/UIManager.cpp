/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIManager.cpp
*/

#include "UI/UIManager.hpp"
#include <algorithm>

namespace zappy {

void UIManager::update(float dt) {
    raylib::Vector2 mousePos = raylib::Mouse::GetPosition();
    auto events = std::make_shared<std::vector<UIEvent>>();

    // Poll mouse events
    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT)) {
        events->push_back({UIEventType::MOUSE_PRESSED_LEFT, mousePos, 0, 0});
    }
    if (raylib::Mouse::IsButtonReleased(MOUSE_BUTTON_LEFT)) {
        events->push_back({UIEventType::MOUSE_RELEASED_LEFT, mousePos, 0, 0});
    }

    // Poll keyboard events
    int key = raylib::Keyboard::GetKeyPressed();
    while (key > 0) {
        events->push_back({UIEventType::KEY_PRESSED, {0, 0}, key, 0});
        key = raylib::Keyboard::GetKeyPressed();
    }

    int charCode = raylib::Keyboard::GetCharPressed();
    while (charCode > 0) {
        events->push_back({UIEventType::CHAR_PRESSED, {0, 0}, 0, charCode});
        charCode = raylib::Keyboard::GetCharPressed();
    }

    // Sort components by zIndex DESCENDING (front-to-back) for event consumption
    std::vector<std::shared_ptr<IUIComponent>> sortedComponents = _components;
    std::sort(sortedComponents.begin(), sortedComponents.end(),
              [](const std::shared_ptr<IUIComponent>& a, const std::shared_ptr<IUIComponent>& b) {
                  return a->getZIndex() > b->getZIndex();
              });

    for (auto& component : sortedComponents) {
        if (component->isVisible()) {
            component->update(dt, mousePos, events);
        }
    }
}

void UIManager::render() {
    // Sort components by zIndex before rendering
    std::vector<std::shared_ptr<IUIComponent>> sortedComponents = _components;
    std::sort(sortedComponents.begin(), sortedComponents.end(),
              [](const std::shared_ptr<IUIComponent>& a, const std::shared_ptr<IUIComponent>& b) {
                  return a->getZIndex() < b->getZIndex();
              });

    for (auto& component : sortedComponents) {
        if (component->isVisible()) {
            component->render();
        }
    }
}

void UIManager::addComponent(std::shared_ptr<IUIComponent> component) {
    _components.push_back(component);
}

void UIManager::clear() { _components.clear(); }

} // namespace zappy
