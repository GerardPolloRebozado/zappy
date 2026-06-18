/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIChatPanel.cpp
*/

#include "UI/UIChatPanel.hpp"
#include "Components/ComponentInhabitant.hpp"
#include <sstream>
namespace zappy {

UIChatPanel::UIChatPanel(raylib::Rectangle bounds, std::shared_ptr<ChatLogs> chatLogs, World& world,
                         int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _chatLogs(chatLogs), _world(world), _fontSize(15),
      _spacing(1.5f) {
    _isVisible = true;
}

void UIChatPanel::update(float dt, raylib::Vector2 mousePos,
                         std::shared_ptr<std::vector<UIEvent>> events) {
    if (raylib::Keyboard::IsKeyPressed(KEY_C)) {
        _isVisible = !_isVisible;
    }
}

void UIChatPanel::render() {
    if (!_isVisible) {
        return;
    }

    _bounds.Draw(raylib::Color{0, 0, 0, 150});

    auto& font = AssetManager::getInstance().getFont("TextFont");
    raylib::Vector2 titlePos = {_bounds.x, _bounds.y - 25}; // Un poco más arriba del fondo
    font.DrawText("World Chat", titlePos, 20, 1.5f, raylib::Color::RayWhite());

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

    float currentY = _bounds.y + 5;

    for (size_t i = startIndex; i < messages.size(); ++i) {
        const auto& msg = messages[i];

        raylib::Color textColor = raylib::Color::RayWhite();

        if (!msg.Team.empty()) {
            textColor = TeamName::findTeam(msg.Team, _world);
        } else {
            auto it = _typeColors.find(msg.Type);
            if (it != _typeColors.end()) {
                textColor = it->second;
            }
        }

        std::string rawText = "[" + msg.Type + "] " + msg.Log;
        std::string displayText = "";
        std::string currentLine = "";

        std::istringstream words(rawText);
        std::string word;
        float maxTextWidth = _bounds.width - 20;

        while (words >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;

            raylib::Vector2 size = font.MeasureText(testLine, _fontSize, _spacing);

            if (size.x > maxTextWidth) {
                displayText += currentLine + "\n";
                currentLine = word;
            } else {
                currentLine = testLine;
            }
        }
        displayText += currentLine;

        raylib::Vector2 pos{_bounds.x + 10, currentY};
        font.DrawText(displayText, pos, _fontSize, _spacing, textColor);
        int lineCount = std::count(displayText.begin(), displayText.end(), '\n') + 1;
        currentY += (_fontSize + 5) * lineCount;
    }
}

} // namespace zappy