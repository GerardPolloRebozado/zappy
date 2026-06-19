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

    // Textured constructor
    UIButton(raylib::Rectangle bounds, const std::string& text, std::function<void()> onClick,
             const std::string& normalTex, const std::string& hoverTex,
             const std::string& pressedTex, int zIndex = 0);

    ~UIButton() override = default;

    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;
    void render() override;

    void setBounds(raylib::Rectangle bounds) override;
    void setZIndex(int zIndex) override;

  private:
    std::unique_ptr<UIText> _label;

    raylib::Color _normalColor;
    raylib::Color _hoverColor;
    raylib::Color _pressedColor;

    std::string _normalTex;
    std::string _hoverTex;
    std::string _pressedTex;
    bool _isTextured;
};

} // namespace zappy

#endif // ZAPPY_UIBUTTON_HPP
