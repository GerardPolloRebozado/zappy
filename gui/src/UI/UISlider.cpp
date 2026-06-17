/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UISlider.cpp
*/

#include "UI/UISlider.hpp"
#include "Components/ComponentShared.hpp"
#include "ECS/World.hpp"
#include "NetworkManager.hpp"

#include <string>

namespace zappy {

UISlider::UISlider(raylib::Rectangle bounds, World& world, NetworkManager& network, int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _world(world), _network(network), _isDragging(false),
      _pendingUpdate(false), _sliderValue(100.0f) {}

/**
 * @brief Updates slider state: syncs from ECS, handles drag events, sends sst on release.
 * @param dt Delta time (unused).
 * @param mousePos Current mouse position in screen coordinates.
 * @param events Shared event vector; consumed events are erased.
 */
void UISlider::update(float dt, raylib::Vector2 mousePos,
                      std::shared_ptr<std::vector<UIEvent>> events) {
    (void)dt;

    // Sync slider value from ECS only when idle (not dragging, no pending server response)
    if (!_isDragging && !_pendingUpdate) {
        auto storage = _world.get_storage<TimeUnit>();
        if (storage) {
            for (auto const& [entity, timeUnit] : *storage) {
                _sliderValue = static_cast<float>(timeUnit->frequency);
                break;
            }
        }
    }

    // Clear pending flag once the ECS reflects our sent value
    if (_pendingUpdate && !_isDragging) {
        auto storage = _world.get_storage<TimeUnit>();
        if (storage) {
            for (auto const& [entity, timeUnit] : *storage) {
                int ecsFreq = timeUnit->frequency;
                if (ecsFreq == static_cast<int>(_sliderValue)) {
                    _pendingUpdate = false;
                }
                break;
            }
        }
    }

    bool isHovered = _bounds.CheckCollision(mousePos);

    // Consume relevant mouse events from the shared event vector
    for (auto it = events->begin(); it != events->end();) {
        bool consumed = false;

        if (it->type == UIEventType::MOUSE_PRESSED_LEFT && isHovered) {
            _isDragging = true;
            _pendingUpdate = false;
            consumed = true;
        }

        if (it->type == UIEventType::MOUSE_RELEASED_LEFT) {
            if (_isDragging) {
                _isDragging = false;
                int freq = static_cast<int>(_sliderValue);
                if (freq < 1) {
                    freq = 1;
                }
                _sliderValue = static_cast<float>(freq);
                _pendingUpdate = true;
                _network.requestTimeUpdate(freq);
                consumed = true;
            }
        }

        if (consumed) {
            it = events->erase(it);
        } else {
            ++it;
        }
    }

    // Map mouse X position within bounds to frequency range [1, 2000]
    if (_isDragging) {
        float relX = mousePos.x - _bounds.x;
        float ratio = relX / _bounds.width;
        if (ratio < 0.0f) {
            ratio = 0.0f;
        }
        if (ratio > 1.0f) {
            ratio = 1.0f;
        }
        _sliderValue = 1.0f + ratio * 1999.0f;
    }
}

/**
 * @brief Renders the slider bar with filled portion and centered label.
 */
void UISlider::render() {
    // Background bar
    _bounds.Draw(raylib::Color(30, 30, 30, 180));

    // Filled portion representing current value
    float fillWidth = (_sliderValue / 2000.0f) * _bounds.width;
    raylib::Rectangle fillRect{_bounds.x, _bounds.y, fillWidth, _bounds.height};
    fillRect.Draw(raylib::Color(0, 150, 255, 200));

    _bounds.DrawLines(raylib::Color(200, 200, 200, 150), 2.0f);

    // Centered label showing current frequency
    std::string label = "Speed: " + std::to_string(static_cast<int>(_sliderValue));
    int fontSize = 16;
    int textWidth = MeasureText(label.c_str(), fontSize);
    int textX = static_cast<int>(_bounds.x + (_bounds.width - textWidth) / 2.0f);
    int textY = static_cast<int>(_bounds.y + (_bounds.height - fontSize) / 2.0f);
    ::DrawText(label.c_str(), textX, textY, fontSize, WHITE);
}

} // namespace zappy
