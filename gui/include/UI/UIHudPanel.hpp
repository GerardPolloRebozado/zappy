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

/**
 * @brief HUD panel displayed when a tile is selected.
 *
 * Shows tile info (biome, resources) and lists players on the tile.
 * Clicking a player row toggles POV follow mode via the FollowingEntity
 * component. Clicking the currently followed player exits POV.
 */
class UIHudPanel : public AUIComponent {
  public:
    UIHudPanel(raylib::Rectangle bounds, World& world, RenderSystem& renderSystem,
               std::function<void()> onClick = nullptr, int zIndex = 0);
    ~UIHudPanel() override = default;

    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;
    void render() override;

  private:
    World& _world;
    RenderSystem& _renderSystem;
};

} // namespace zappy

#endif // ZAPPY_UIHUDPANEL_HPP
