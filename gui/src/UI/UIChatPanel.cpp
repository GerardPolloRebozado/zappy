/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIChatPanel.cpp
*/

#include "UI/UIChatPanel.hpp"

namespace zappy {

UIChatPanel::UIChatPanel(raylib::Rectangle bounds, std::shared_ptr<ChatLogs> chatLogs, int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _chatLogs(chatLogs), _fontSize(15), _spacing(1.5f) {}
void UIChatPanel::update(float dt, raylib::Vector2 mousePos,
                         std::shared_ptr<std::vector<UIEvent>> events) {
    (void)dt;
    (void)mousePos;
    (void)events;
}

void UIChatPanel::render() {
    _bounds.Draw(raylib::Color{0, 0, 0, 150});

    if (!_chatLogs) {
        return;
    }

    const auto& messages = _chatLogs->getLogs();
    if (messages.empty()) {
        return;
    }

    int maxLines = static_cast<int>(_bounds.height / (_fontSize + 5));

    size_t startIndex = 0;
    if (messages.size() > static_cast<size_t>(maxLines)) {
        startIndex = messages.size() - maxLines;
    }

    auto& font = AssetManager::getInstance().getFont("TextFont");

    float currentY = _bounds.y + 5;

    for (size_t i = startIndex; i < messages.size(); ++i) {
        const auto& msg = messages[i];

        raylib::Color textColor = raylib::Color::RayWhite();
        if (msg.Type == "ERROR" || msg.Type == "DEATH") {
            textColor = raylib::Color::Red();
        } else if (msg.Type == "JOIN") {
            textColor = raylib::Color::Green();
        } else if (msg.Type == "CHAT" || msg.Type == "BROADCAST") {
            textColor = raylib::Color::Yellow();
        }

        std::string displayText = "[" + msg.Type + "] " + msg.Log;

        raylib::Vector2 pos{_bounds.x + 10, currentY};
        font.DrawText(displayText, pos, _fontSize, _spacing, textColor);

        currentY += (_fontSize + 5);
    }
}

} // namespace zappy