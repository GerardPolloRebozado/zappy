/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIManager.cpp
*/

#include "UI/UIManager.hpp"
#include <algorithm>
#include <raylib.h>

namespace zappy {

void UIManager::update(float dt) {
    Vector2 mousePos = GetMousePosition();

    // Copy to allow components to safely modify the list (like clear() on click)
    auto componentsCopy = _components;
    for (auto& component : componentsCopy) {
        if (component->isVisible()) {
            component->update(dt, mousePos);
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
