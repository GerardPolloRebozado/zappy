/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIHudPanel.hpp
*/

#ifndef ZAPPY_UIHUDPANEL_HPP
#define ZAPPY_UIHUDPANEL_HPP

#include "ECS/World.hpp"
#include "Systems/RenderSystem.hpp"
#include "UI/AUIComponent.hpp"

namespace zappy {

class UIHudPanel : public AUIComponent {
  public:
    UIHudPanel(raylib::Rectangle bounds, World& world, const RenderSystem& renderSystem,
               int zIndex = 0);
    ~UIHudPanel() override = default;

    void render() override;

  private:
    World& _world;
    const RenderSystem& _renderSystem;
};

} // namespace zappy

#endif // ZAPPY_UIHUDPANEL_HPP
