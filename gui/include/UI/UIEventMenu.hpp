/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIEventMenu.hpp
*/

#ifndef ZAPPY_UIEVENTMENU_HPP
#define ZAPPY_UIEVENTMENU_HPP

#include "AUIComponent.hpp"
#include "ECS/World.hpp"
#include "Logging/ChatLogs.hpp"
#include "UIButton.hpp"
#include "UISlider.hpp"
#include <array>
#include <functional>
#include <memory>
#include <string>

namespace zappy {

class NetworkManager;

/**
 * @brief Bottom-right toolbar that toggles a popup with map-event buttons and
 *        the time-frequency slider.
 *
 * When collapsed only the toggle button is visible. When expanded a
 * Minecraft-style panel (9-patch @c menu_bg) appears above the button
 * containing a 2x2 grid of event icons and the frequency slider.
 *
 * Each event button checks the cached @c MapEvent ECS component:
 * - If no event is active, sends @c mev to trigger the requested event.
 * - If an event is already running, posts a warning to the chat log.
 */
class UIEventMenu : public AUIComponent {
  public:
    UIEventMenu(raylib::Rectangle bounds, World& world, NetworkManager& network,
                std::shared_ptr<ChatLogs> chatLogs, int zIndex = 0);
    ~UIEventMenu() override = default;

    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;
    void render() override;

  private:
    struct EventDef {
        std::string name;
        std::string texKey;
        std::string label;
    };

    void _tryTriggerEvent(const std::string& eventName);
    void _drawNPatchBackground(raylib::Rectangle rect);

    World& _world;
    NetworkManager& _network;
    std::shared_ptr<ChatLogs> _chatLogs;

    bool _isOpen;
    std::unique_ptr<UIButton> _toggleButton;
    std::array<std::unique_ptr<UIButton>, 4> _eventButtons;
    std::unique_ptr<UISlider> _slider;

    raylib::Rectangle _popupBounds;
    static const std::array<EventDef, 4> EVENT_DEFS;
};

} // namespace zappy

#endif // ZAPPY_UIEVENTMENU_HPP
