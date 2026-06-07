/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIButton.hpp
*/

#ifndef ZAPPY_UIBUTTON_HPP
#define ZAPPY_UIBUTTON_HPP

#include "AUIComponent.hpp"
#include "UIText.hpp"
#include <functional>
#include <memory>
#include <string>

namespace zappy {

class UIButton : public AUIComponent {
  public:
    UIButton(raylib::Rectangle bounds, const std::string& text, std::function<void()> onClick,
             int zIndex = 0);
    ~UIButton() override = default;

    void update(float dt, raylib::Vector2 mousePos) override;
    void render() override;

    void setBounds(raylib::Rectangle bounds) override;
    void setZIndex(int zIndex) override;

  private:
    std::unique_ptr<UIText> _label;
    std::function<void()> _onClick;
    bool _isHovered;
    bool _isPressed;

    raylib::Color _normalColor;
    raylib::Color _hoverColor;
    raylib::Color _pressedColor;
};

} // namespace zappy

#endif // ZAPPY_UIBUTTON_HPP
