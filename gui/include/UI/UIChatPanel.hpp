/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIChatPanel.hpp
*/

#ifndef ZAPPY_UICHATPANEL_HPP
#define ZAPPY_UICHATPANEL_HPP

#include "AUIComponent.hpp"
#include "Graphics/AssetManager.hpp"
#include "Logging/ChatLogs.hpp"
#include <memory>

namespace zappy {

class UIChatPanel : public AUIComponent {
  public:
    UIChatPanel(raylib::Rectangle bounds, std::shared_ptr<ChatLogs> chatLogs, int zIndex = 0);
    ~UIChatPanel() override = default;

    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;
    void render() override;

  private:
    std::shared_ptr<ChatLogs> _chatLogs;
    int _fontSize;
    float _spacing;
};

} // namespace zappy

#endif // ZAPPY_UICHATPANEL_HPP