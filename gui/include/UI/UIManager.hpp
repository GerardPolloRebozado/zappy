/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIManager.hpp
*/

#ifndef ZAPPY_UIMANAGER_HPP
#define ZAPPY_UIMANAGER_HPP

#include "IUIComponent.hpp"
#include <memory>
#include <vector>

namespace zappy {

class UIManager {
  public:
    UIManager() = default;
    ~UIManager() = default;

    void update(float dt);
    void render();

    void addComponent(std::shared_ptr<IUIComponent> component);
    void clear();

  private:
    std::vector<std::shared_ptr<IUIComponent>> _components;
};

} // namespace zappy

#endif // ZAPPY_UIMANAGER_HPP
