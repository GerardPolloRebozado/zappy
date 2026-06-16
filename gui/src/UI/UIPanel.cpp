/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIPanel.cpp
*/

#include "UI/UIPanel.hpp"

namespace zappy {

UIPanel::UIPanel(raylib::Rectangle bounds, raylib::Color color, int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _color(color) {}

void UIPanel::render() { _bounds.Draw(_color); }

} // namespace zappy
