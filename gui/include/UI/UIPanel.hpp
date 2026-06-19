/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIPanel.hpp
*/

#ifndef ZAPPY_UIPANEL_HPP
#define ZAPPY_UIPANEL_HPP

#include "AUIComponent.hpp"

namespace zappy {

class UIPanel : public AUIComponent {
  public:
    UIPanel(raylib::Rectangle bounds, raylib::Color color, int zIndex = 0);
    UIPanel(raylib::Rectangle bounds, const std::string& textureName, int zIndex = 0);
    ~UIPanel() override = default;

    void render() override;

  private:
    raylib::Color _color;
    std::string _textureName;
    bool _isTextured;
};

} // namespace zappy

#endif // ZAPPY_UIPANEL_HPP
