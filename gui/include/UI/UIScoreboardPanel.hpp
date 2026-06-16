/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIScoreboardPanel.hpp
*/

#ifndef ZAPPY_UISCOREBOARDPANEL_HPP
#define ZAPPY_UISCOREBOARDPANEL_HPP

#include "ECS/World.hpp"
#include "UI/AUIComponent.hpp"
#include "UI/UIManager.hpp"
#include <memory>

namespace zappy {
class UIScoreboardPanel : public AUIComponent {
  public:
    UIScoreboardPanel(raylib::Rectangle bounds, World& world, int zIndex = 0);
    ~UIScoreboardPanel() override = default;

    void render() override;

  private:
    World& _world;
};

} // namespace zappy

#endif // ZAPPY_UISCOREBOARDPANEL_HPP
