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
      _placeholderColor(raylib::Color::Gray()), _isTextured(false) {}

UIInput::UIInput(raylib::Rectangle bounds, const std::string& initialText,
                 const std::string& placeholder, const std::string& textureName, size_t maxLength,
                 int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _text(initialText), _placeholder(placeholder),
      _maxLength(maxLength), _isFocused(false), _normalColor(raylib::Color::White()),
      _focusedColor(raylib::Color::White()), _textColor(raylib::Color::RayWhite()),
      _placeholderColor(raylib::Color::Gray()), _textureName(textureName), _isTextured(true) {}

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
    if (_isTextured) {
        auto& tex = AssetManager::getInstance().getTexture(_textureName);
        if (tex.id != 0) {
            raylib::Rectangle sourceRec(0.0f, 0.0f, (float)tex.width, (float)tex.height);
            // Slight tint when not focused so it looks "inactive", full bright when focused
            raylib::Color tint =
                _isFocused ? raylib::Color::White() : raylib::Color(200, 200, 200, 255);
            tex.Draw(sourceRec, _bounds, raylib::Vector2(0, 0), 0.0f, tint);
            if (_isFocused) {
                _bounds.DrawLines(raylib::Color(255, 255, 255, 100), 2.0f);
            }
        }
    } else {
        raylib::Color bgColor = _isFocused ? _focusedColor : _normalColor;
        _bounds.Draw(bgColor);
        _bounds.DrawLines(raylib::Color(0, 100, 255, 255), _isFocused ? 3.0f : 1.0f);
    }

    int paddingX = 25; // increased horizontal padding
    std::string displayTxt = _text.empty() && !_isFocused ? _placeholder : _text;
    raylib::Color txtColor = _text.empty() && !_isFocused ? _placeholderColor : _textColor;

    if (_isFocused && (int)(GetTime() * 2) % 2 == 0) {
        displayTxt += "_";
    }

    raylib::Text(displayTxt, 20, txtColor, AssetManager::getInstance().getFont("TextFont"), 1.5f)
        .Draw(_bounds.x + paddingX, _bounds.y + _bounds.height / 2 - 10);
}

std::string UIInput::getText() const { return _text; }

} // namespace zappy
