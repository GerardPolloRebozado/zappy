/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIEvent.hpp
*/

#ifndef ZAPPY_UIEVENT_HPP
#define ZAPPY_UIEVENT_HPP

#include <raylib-cpp.hpp>

namespace zappy {

enum class UIEventType { MOUSE_PRESSED_LEFT, MOUSE_RELEASED_LEFT, KEY_PRESSED, CHAR_PRESSED };

struct UIEvent {
    UIEventType type;
    raylib::Vector2 mousePos;
    int keyCode;
    int charCode;
};

} // namespace zappy

#endif // ZAPPY_UIEVENT_HPP
