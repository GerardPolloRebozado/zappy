/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** UiRadio.cpp
*/
#include "UI/UIRadio.hpp"
#include "Components/ComponentMusic.hpp"
#include <raylib.h>

namespace zappy {

UIRadio::UIRadio(raylib::Rectangle bounds, World& world, int zIndex, int songIdx,
                 std::vector<std::string> playlist)
    : AUIComponent(bounds, nullptr, zIndex), _songIdxM(songIdx), _world(world),
      _playlist(playlist) {

    _isVisible = true;

    _menuRadio = std::make_shared<UIPanel>(
        raylib::Rectangle{bounds.x + 10, bounds.y + 10, 250.0f, 125.0f}, "input_bg", zIndex);

    _closeRadio = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 205.0f, bounds.y + 15.0f, 40.0f, 40.0f}, "",
        [this]() {
            opened = false;
            this->setBounds(raylib::Rectangle{_bounds.x, _bounds.y, 160.0f, 100.0f});
        },
        "cross", "cross", "cross", zIndex + 1);

    _prevSong = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 40.0f, bounds.y + 65.0f, 40.0f, 40.0f}, "",
        [this]() {
            if (_playlist.empty()) {
                return;
            }

            if (_songIdxM == 0) {
                _songIdxM = _playlist.size() - 1;
            } else {
                _songIdxM--;
            }

            auto storage = _world.get_storage<ComponentMusic>();
            if (storage) {
                for (auto& [ent, comp] : *storage) {
                    _world.despawn(ent);
                    break;
                }
            }
            auto newMusicEnt = _world.spawn();
            _world.add_component(newMusicEnt,
                                 std::make_shared<ComponentMusic>(_playlist[_songIdxM], true));
        },
        "prev", "prev", "prev", zIndex + 1);

    _playBtn = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 90.0f, bounds.y + 65.0f, 40.0f, 40.0f}, "",
        [this]() {
            auto storage = _world.get_storage<ComponentMusic>();
            if (storage) {
                for (auto& [ent, comp] : *storage) {
                    if (!comp->isPlaying && comp->isStarted) {
                        comp->isPlaying = true;
                        ResumeMusicStream(comp->song.music);
                    }
                    break;
                }
            }
        },
        "play", "play", "play", zIndex + 1);

    _stopBtn = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 140.0f, bounds.y + 65.0f, 40.0f, 40.0f}, "",
        [this]() {
            auto storage = _world.get_storage<ComponentMusic>();
            if (storage) {
                for (auto& [ent, comp] : *storage) {
                    if (comp->isPlaying) {
                        comp->isPlaying = false;
                        PauseMusicStream(comp->song.music);
                    }
                    break;
                }
            }
        },
        "stop", "stop", "stop", zIndex + 1);

    _nextSong = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x + 190.0f, bounds.y + 65.0f, 40.0f, 40.0f}, "",
        [this]() {
            if (_playlist.empty()) {
                return;
            }
            _songIdxM++;
            if (_songIdxM >= _playlist.size()) {
                _songIdxM = 0;
            }
            auto storage = _world.get_storage<ComponentMusic>();
            if (storage) {
                for (auto& [ent, comp] : *storage) {
                    _world.despawn(ent);
                    break;
                }
            }
            auto newMusicEnt = _world.spawn();
            _world.add_component(newMusicEnt,
                                 std::make_shared<ComponentMusic>(_playlist[_songIdxM], true));
        },
        "next", "next", "next", zIndex + 1);

    _openRadio = std::make_shared<UIButton>(
        raylib::Rectangle{bounds.x, bounds.y, 160.0f, 100.0f}, "",
        [this]() {
            opened = true;
            this->setBounds(raylib::Rectangle{_bounds.x, _bounds.y, 270.0f, 120.0f});
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
        if (_prevSong) {
            _prevSong->update(dt, mousePos, events);
        }
        if (_playBtn) {
            _playBtn->update(dt, mousePos, events);
        }
        if (_stopBtn) {
            _stopBtn->update(dt, mousePos, events);
        }
        if (_nextSong) {
            _nextSong->update(dt, mousePos, events);
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
        if (_prevSong) {
            _prevSong->render();
        }
        if (_playBtn) {
            _playBtn->render();
        }
        if (_stopBtn) {
            _stopBtn->render();
        }
        if (_nextSong) {
            _nextSong->render();
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