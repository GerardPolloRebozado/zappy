/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIButton.cpp
*/

#include "UI/UIButton.hpp"
#include "UI/AUIComponent.hpp"

#include "Graphics/AssetManager.hpp"

namespace zappy {

UIButton::UIButton(raylib::Rectangle bounds, const std::string& text, std::function<void()> onClick,
                   int zIndex)
    : AUIComponent(bounds, onClick, zIndex), _normalColor(raylib::Color(0, 80, 200, 255)),
      _hoverColor(raylib::Color(0, 100, 255, 255)), _pressedColor(raylib::Color(0, 50, 150, 255)),
      _isTextured(false) {
    _label = std::make_unique<UIText>(bounds, text, 20, raylib::Color::RayWhite(), zIndex, 1.5f);
}

UIButton::UIButton(raylib::Rectangle bounds, const std::string& text, std::function<void()> onClick,
                   const std::string& normalTex, const std::string& hoverTex,
                   const std::string& pressedTex, int zIndex)
    : AUIComponent(bounds, onClick, zIndex), _normalColor(raylib::Color::White()),
      _hoverColor(raylib::Color::White()), _pressedColor(raylib::Color::White()),
      _normalTex(normalTex), _hoverTex(hoverTex), _pressedTex(pressedTex), _isTextured(true) {
    _label = std::make_unique<UIText>(bounds, text, 20, raylib::Color::RayWhite(), zIndex, 1.5f);
}

void UIButton::update(float dt, raylib::Vector2 mousePos,
                      std::shared_ptr<std::vector<UIEvent>> events) {
    AUIComponent::update(dt, mousePos, events);
}

void UIButton::render() {
    raylib::Rectangle drawBounds = _bounds;
    if (_isPressed) {
        drawBounds.y += 4.0f; // Visual feedback for press
    }

    if (_isTextured) {
        std::string texToUse = _normalTex;
        raylib::Color tint = raylib::Color::White();
        if (_isPressed) {
            texToUse = _pressedTex;
            tint = raylib::Color(200, 200, 200, 255); // Slightly darken
        } else if (_isHovered) {
            texToUse = _hoverTex;
        }

        auto& tex = AssetManager::getInstance().getTexture(texToUse);
        if (tex.id != 0) {
            raylib::Rectangle sourceRec(0.0f, 0.0f, (float)tex.width, (float)tex.height);
            tex.Draw(sourceRec, drawBounds, raylib::Vector2(0, 0), 0.0f, tint);
        }
    } else {
        raylib::Color btnColor = _normalColor;
        if (_isPressed) {
            btnColor = _pressedColor;
        } else if (_isHovered) {
            btnColor = _hoverColor;
        }

        drawBounds.Draw(btnColor);
        drawBounds.DrawLines(raylib::Color(200, 200, 255, 100), 2.0f);
    }

    if (_label) {
        if (_isPressed) {
            raylib::Rectangle labelBounds = _label->getBounds();
            _label->setBounds(raylib::Rectangle{labelBounds.x, labelBounds.y + 4.0f,
                                                labelBounds.width, labelBounds.height});
            _label->render();
            _label->setBounds(labelBounds); // Restore original bounds
        } else {
            _label->render();
        }
    }
}

void UIButton::setBounds(raylib::Rectangle bounds) {
    _bounds = bounds;
    if (_label) {
        _label->setBounds(bounds);
    }
}

void UIButton::setZIndex(int zIndex) {
    _zIndex = zIndex;
    if (_label) {
        _label->setZIndex(zIndex);
    }
}

} // namespace zappy
