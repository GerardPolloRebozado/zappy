/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** IUIComponent.hpp
*/

#ifndef ZAPPY_IUICOMPONENT_HPP
#define ZAPPY_IUICOMPONENT_HPP

#include <raylib-cpp.hpp>

namespace zappy {

class IUIComponent {
  public:
    virtual ~IUIComponent() = default;

    virtual void update(float dt, raylib::Vector2 mousePos) = 0;
    virtual void render() = 0;

    virtual int getZIndex() const = 0;
    virtual void setZIndex(int zIndex) = 0;

    virtual raylib::Rectangle getBounds() const = 0;
    virtual void setBounds(raylib::Rectangle bounds) = 0;

    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;
};

} // namespace zappy

#endif // ZAPPY_IUICOMPONENT_HPP
