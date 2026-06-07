/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIInput.cpp
*/

#include "UI/UIInput.hpp"
#include <raylib.h>

namespace zappy {

UIInput::UIInput(raylib::Rectangle bounds, const std::string& initialText,
                 const std::string& placeholder, size_t maxLength, int zIndex)
    : AUIComponent(bounds, zIndex), _text(initialText), _placeholder(placeholder),
      _maxLength(maxLength), _isFocused(false), _normalColor(raylib::Color::LightGray()),
      _focusedColor(raylib::Color::White()), _textColor(raylib::Color::Black()),
      _placeholderColor(GRAY) {}

void UIInput::update(float dt, raylib::Vector2 mousePos) {
    (void)dt;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        _isFocused = CheckCollisionPointRec(mousePos, _bounds);
    }

    if (_isFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (_text.size() < _maxLength)) {
                _text += (char)key;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (!_text.empty()) {
                _text.pop_back();
            }
        }
    }
}

void UIInput::render() {
    raylib::Color bgColor = _isFocused ? _focusedColor : _normalColor;
    DrawRectangleRec(_bounds, bgColor);
    DrawRectangleLinesEx(_bounds, _isFocused ? 3.0f : 1.0f, DARKGRAY);

    int padding = 5;
    std::string displayTxt = _text.empty() && !_isFocused ? _placeholder : _text;
    raylib::Color txtColor = _text.empty() && !_isFocused ? _placeholderColor : _textColor;

    ::DrawText(displayTxt.c_str(), (int)(_bounds.x) + padding,
               (int)(_bounds.y + _bounds.height / 2) - 10, 20, txtColor);
}

std::string UIInput::getText() const { return _text; }

} // namespace zappy
