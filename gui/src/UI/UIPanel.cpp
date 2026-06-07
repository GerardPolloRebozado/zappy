/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIPanel.cpp
*/

#include "UI/UIPanel.hpp"
#include <raylib.h>

namespace zappy {

UIPanel::UIPanel(raylib::Rectangle bounds, raylib::Color color, int zIndex)
    : AUIComponent(bounds, zIndex), _color(color) {}

void UIPanel::render() { DrawRectangleRec(_bounds, _color); }

} // namespace zappy
