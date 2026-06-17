/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UISlider.hpp
*/

#ifndef ZAPPY_UISLIDER_HPP
#define ZAPPY_UISLIDER_HPP

#include "UI/AUIComponent.hpp"

namespace zappy {

class NetworkManager;
class World;

/**
 * @brief Draggable slider to control the server time frequency (sst command).
 *
 * Reads the current frequency from the ECS TimeUnit component and sends
 * an sst request to the server when the user releases the slider.
 * Range: [1, 2000].
 */
class UISlider : public AUIComponent {
  public:
    /**
     * @brief Construct a new UISlider.
     * @param bounds Screen rectangle for the slider bar.
     * @param world ECS world used to read the current TimeUnit frequency.
     * @param network Network manager used to send sst commands.
     * @param zIndex Rendering/update priority.
     */
    UISlider(raylib::Rectangle bounds, World& world, NetworkManager& network, int zIndex = 0);
    ~UISlider() override = default;

    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;
    void render() override;

  private:
    World& _world;
    NetworkManager& _network;
    bool _isDragging;
    bool _pendingUpdate;
    float _sliderValue;
};

} // namespace zappy

#endif // ZAPPY_UISLIDER_HPP
