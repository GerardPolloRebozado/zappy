/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIPanel.cpp
*/

#include "UI/UIPanel.hpp"

#include "Graphics/AssetManager.hpp"

namespace zappy {

UIPanel::UIPanel(raylib::Rectangle bounds, raylib::Color color, int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _color(color), _isTextured(false) {}

UIPanel::UIPanel(raylib::Rectangle bounds, const std::string& textureName, int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _color(raylib::Color::White()),
      _textureName(textureName), _isTextured(true) {}

void UIPanel::render() {
    if (_isTextured) {
        auto& tex = AssetManager::getInstance().getTexture(_textureName);
        if (tex.id != 0) {
            raylib::Rectangle sourceRec(0.0f, 0.0f, (float)tex.width, (float)tex.height);
            tex.Draw(sourceRec, _bounds, raylib::Vector2(0, 0), 0.0f, raylib::Color::White());
        }
    } else {
        _bounds.Draw(_color);
    }
}

} // namespace zappy
