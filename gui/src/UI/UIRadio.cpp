/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** UiRadio.cpp
*/
#include "UI/UIRadio.hpp"
#include "Graphics/AssetManager.hpp"

namespace zappy {

UIRadio::UIRadio(raylib::Rectangle bounds, World& world, int zIndex, int songIdx)
    : AUIComponent(bounds, nullptr, zIndex), _songIdxM(songIdx), _world(world) {
    _isVisible = true;

    _menuRadio = std::make_shared<UIPanel>(
        raylib::Rectangle{bounds.x + 10, bounds.y + 10, 150.0f, 100.0f}, "input_bg", zIndex);

    _closeRadio = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 120, bounds.y + 10, 30.0f, 30.0f}, "",
        [this]() {
            opened = false;
            this->setBounds(raylib::Rectangle{_bounds.x, _bounds.y, 40.0f, 40.0f});
        },
        "cross", "cross", "cross", zIndex + 1);

    _nextSong = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 60, bounds.y + 50, 30.0f, 30.0f}, "",
        [this]() {
            _songIdxM++;
            if (_songIdxM >= AssetManager::getInstance().getMusicPath("songs").size()) {
                _songIdxM = 0;
            }
            // TODO: change world ind to the song
        },
        "play", "play", "play", zIndex + 1);

    _prevSong = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 20, bounds.y + 50, 30.0f, 30.0f}, "",
        [this]() {
            if (_songIdxM == 0) {
                _songIdxM = AssetManager::getInstance().getMusicPath("songs").size() - 1;
            } else {
                _songIdxM--;
            }
            // TODO: change world ind to the song
        },
        "stop", "stop", "stop", zIndex + 1);

    _openRadio = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x, bounds.y, 40.0f, 40.0f}, "",
        [this]() {
            opened = true;
            this->setBounds(raylib::Rectangle{_bounds.x, _bounds.y, 160.0f, 110.0f});
        },
        "laud", "laud", "laud", zIndex + 2);
}

void UIRadio::update(float dt, raylib::Vector2 mousePos,
                     std::shared_ptr<std::vector<UIEvent>> events) {
    if (!opened) {
        if (_openRadio) {
            _openRadio->update(dt, mousePos, events);
        }
    } else {
        if (_closeRadio) {
            _closeRadio->update(dt, mousePos, events);
        }
        if (_nextSong) {
            _nextSong->update(dt, mousePos, events);
        }
        if (_prevSong) {
            _prevSong->update(dt, mousePos, events);
        }
    }

    AUIComponent::update(dt, mousePos, events);
}

void UIRadio::render() {
    if (!_isVisible) {
        return;
    }

    if (!opened) {
        if (_openRadio) {
            _openRadio->render();
        }
    } else {
        if (_menuRadio) {
            _menuRadio->render();
        }
        if (_closeRadio) {
            _closeRadio->render();
        }
        if (_nextSong) {
            _nextSong->render();
        }
        if (_prevSong) {
            _prevSong->render();
        }
    }
}

int UIRadio::getZIndex() const { return AUIComponent::getZIndex(); }
void UIRadio::setZIndex(int zIndex) { AUIComponent::setZIndex(zIndex); }
raylib::Rectangle UIRadio::getBounds() const { return AUIComponent::getBounds(); }
void UIRadio::setBounds(raylib::Rectangle bounds) { AUIComponent::setBounds(bounds); }
bool UIRadio::isVisible() const { return AUIComponent::isVisible(); }
void UIRadio::setVisible(bool visible) { AUIComponent::setVisible(visible); }

} // namespace zappy