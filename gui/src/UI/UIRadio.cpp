#include "UI/UIRadio.hpp"
#include "Graphics/AssetManager.hpp"

namespace zappy {

UIRadio::UIRadio(raylib::Rectangle bounds, World& world, int zIndex, int songIdx)
    : AUIComponent(bounds, nullptr, zIndex), _songIdxM(songIdx), _world(world) {
    _isVisible = true;

    _menuRadio = std::make_shared<UIPanel>(
        raylib::Rectangle{bounds.x + 10, bounds.y + 10, _bounds.width, _bounds.height},
        "input_background", zIndex);

    _nextSong = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 60, bounds.y + 50, 30, 30}, "",
        [this]() {
            _songIdxM++;
            if (_songIdxM >= AssetManager::getInstance().getMusicPath("songs").size()) {
                _songIdxM = 0;
            }
            // TODO: change world ind to the song
        },
        "play", "play", "play", zIndex + 1);

    _prevSong = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 20, bounds.y + 50, 30, 30}, "",
        [this]() {
            if (_songIdxM == 0) {
                _songIdxM = AssetManager::getInstance().getMusicPath("songs").size() - 1;
            } else {
                _songIdxM--;
            }
            // TODO: change world ind to the song
        },
        "play", "play", "play", zIndex + 1);

    _openRadio = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x, bounds.y, bounds.width, bounds.height}, "",
        [this]() { opened = !opened; }, "laud", "laud", "laud", zIndex + 2);
}

void UIRadio::update(float dt, raylib::Vector2 mousePos,
                     std::shared_ptr<std::vector<UIEvent>> events) {
    AUIComponent::update(dt, mousePos, events);

    if (_openRadio) {
        _openRadio->update(dt, mousePos, events);
    }

    if (opened) {
        if (_nextSong) {
            _nextSong->update(dt, mousePos, events);
        }
        if (_prevSong) {
            _prevSong->update(dt, mousePos, events);
        }
    }
}

void UIRadio::render() {
    if (!_isVisible) {
        return;
    }

    if (_openRadio) {
        _openRadio->render();
    }

    if (opened) {
        if (_menuRadio) {
            _menuRadio->render();
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