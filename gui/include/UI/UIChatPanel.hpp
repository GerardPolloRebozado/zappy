/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIChatPanel.hpp
*/

#ifndef ZAPPY_UICHATPANEL_HPP
#define ZAPPY_UICHATPANEL_HPP

#include "AUIComponent.hpp"
#include "ECS/World.hpp"
#include "Graphics/AssetManager.hpp"
#include "Logging/ChatLogs.hpp"
#include <map>
#include <memory>
#include <string>

namespace zappy {

/**
 * @class UIChatPanel
 * @brief Is a in-game chat window.
 * * This component listens to the global ChatLogs and displays the messages
 */
class UIChatPanel : public AUIComponent {
  public:
    /**
     * @brief Constructs a new UIChatPanel.
     * * @param bounds The physical boundaries (x, y, width, height) of the panel on the screen.
     * @param chatLogs A shared pointer to the data layer containing the message history.
     * @param world A reference to the ECS World, used to fetch dynamic team colors.
     * @param zIndex The rendering layer depth. Higher values are rendered on top.
     */
    UIChatPanel(raylib::Rectangle bounds, std::shared_ptr<ChatLogs> chatLogs, World& world,
                int zIndex = 0);

    ~UIChatPanel() override = default;
    // for the future scroll chat
    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;

    /**
     * @brief Renders the chat panel and its text to the screen.
     * Draws the background and iterates through the visible logs, applying the correct
     * color based on the event type or the team color.
     */
    void render() override;

  private:
    std::shared_ptr<ChatLogs> _chatLogs;
    World& _world;
    int _fontSize;
    float _spacing;
    /**
     * @brief Dictionary of colors for system events without a team.
     */
    std::map<std::string, raylib::Color> _typeColors = {
        {"ERROR", raylib::Color::Red()},        {"DEATH", raylib::Color::Red()},
        {"JOIN", raylib::Color::Green()},       {"CHAT", raylib::Color::Yellow()},
        {"BROADCAST", raylib::Color::Yellow()},
    };
};

} // namespace zappy

#endif // ZAPPY_UICHATPANEL_HPP