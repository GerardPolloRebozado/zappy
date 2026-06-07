/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIInput.hpp
*/

#ifndef ZAPPY_UIINPUT_HPP
#define ZAPPY_UIINPUT_HPP

#include "AUIComponent.hpp"
#include <string>

namespace zappy {

class UIInput : public AUIComponent {
  public:
    UIInput(raylib::Rectangle bounds, const std::string& initialText,
            const std::string& placeholder, size_t maxLength = 256, int zIndex = 0);
    ~UIInput() override = default;

    void update(float dt, raylib::Vector2 mousePos) override;
    void render() override;

    std::string getText() const;

  private:
    std::string _text;
    std::string _placeholder;
    size_t _maxLength;
    bool _isFocused;

    raylib::Color _normalColor;
    raylib::Color _focusedColor;
    raylib::Color _textColor;
    raylib::Color _placeholderColor;
};

} // namespace zappy

#endif // ZAPPY_UIINPUT_HPP
