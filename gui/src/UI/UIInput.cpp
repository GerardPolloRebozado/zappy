/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIInput.cpp
*/

#include "UI/UIInput.hpp"
#include "Graphics/AssetManager.hpp"

namespace zappy {

UIInput::UIInput(raylib::Rectangle bounds, const std::string& initialText,
                 const std::string& placeholder, size_t maxLength, int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _text(initialText), _placeholder(placeholder),
      _maxLength(maxLength), _isFocused(false), _normalColor(raylib::Color(25, 30, 50, 255)),
      _focusedColor(raylib::Color(35, 45, 75, 255)), _textColor(raylib::Color::RayWhite()),
      _placeholderColor(raylib::Color::Gray()) {}

void UIInput::update(float dt, raylib::Vector2 mousePos,
                     std::shared_ptr<std::vector<UIEvent>> events) {
    (void)dt;

    for (auto it = events->begin(); it != events->end();) {
        bool consumed = false;

        if (it->type == UIEventType::MOUSE_PRESSED_LEFT) {
            _isFocused = _bounds.CheckCollision(mousePos);
            if (_isFocused) {
                consumed = true;
            }
        }

        if (_isFocused) {
            if (it->type == UIEventType::CHAR_PRESSED) {
                int charCode = it->charCode;
                if ((charCode >= 32) && (charCode <= 125) && (_text.size() < _maxLength)) {
                    _text += (char)charCode;
                }
                consumed = true;
            }

            if (it->type == UIEventType::KEY_PRESSED) {
                if (it->keyCode == KEY_BACKSPACE) {
                    if (!_text.empty()) {
                        _text.pop_back();
                    }
                }
                consumed = true;
            }
        }

        if (consumed) {
            it = events->erase(it);
        } else {
            ++it;
        }
    }
}

void UIInput::render() {
    raylib::Color bgColor = _isFocused ? _focusedColor : _normalColor;
    _bounds.Draw(bgColor);
    _bounds.DrawLines(raylib::Color(0, 100, 255, 255), _isFocused ? 3.0f : 1.0f);

    int padding = 5;
    std::string displayTxt = _text.empty() && !_isFocused ? _placeholder : _text;
    raylib::Color txtColor = _text.empty() && !_isFocused ? _placeholderColor : _textColor;

    raylib::Text(displayTxt, 20, txtColor, AssetManager::getInstance().getFont("BoldPixels"), 1.5f)
        .Draw(_bounds.x + padding, _bounds.y + _bounds.height / 2 - 10);
}

std::string UIInput::getText() const { return _text; }

} // namespace zappy
