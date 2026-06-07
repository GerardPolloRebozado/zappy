/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** IUIComponent.hpp
*/

#ifndef ZAPPY_IUICOMPONENT_HPP
#define ZAPPY_IUICOMPONENT_HPP

#include "UIEvent.hpp"
#include <memory>
#include <raylib-cpp.hpp>
#include <vector>

namespace zappy {

/**
 * @interface IUIComponent
 * @brief Base interface for all Object-Oriented UI components in the Zappy GUI.
 *
 * This interface dictates the lifecycle and properties required by the UIManager
 * to orchestrate user interface elements properly (rendering, updating, and sorting).
 */
class IUIComponent {
  public:
    virtual ~IUIComponent() = default;

    /**
     * @brief Updates the component's internal state and handles events.
     * @param dt The time elapsed since the last frame (delta time).
     * @param mousePos The current 2D position of the mouse cursor.
     * @param events A shared pointer to the list of UI events for the current frame.
     */
    virtual void update(float dt, raylib::Vector2 mousePos,
                        std::shared_ptr<std::vector<UIEvent>> events) = 0;

    /**
     * @brief Renders the component to the screen.
     */
    virtual void render() = 0;

    /**
     * @brief Gets the z-index of the component.
     * Higher values are rendered on top of lower values.
     * @return int The z-index.
     */
    virtual int getZIndex() const = 0;

    /**
     * @brief Sets the z-index of the component.
     * @param zIndex The new z-index.
     */
    virtual void setZIndex(int zIndex) = 0;

    /**
     * @brief Gets the bounding box of the component.
     * @return raylib::Rectangle The rectangle representing position and size.
     */
    virtual raylib::Rectangle getBounds() const = 0;

    /**
     * @brief Sets the bounding box of the component.
     * @param bounds The new bounds.
     */
    virtual void setBounds(raylib::Rectangle bounds) = 0;

    /**
     * @brief Checks if the component is currently visible.
     * @return true If the component is visible and should be updated/rendered.
     * @return false If the component is hidden.
     */
    virtual bool isVisible() const = 0;

    /**
     * @brief Sets the visibility of the component.
     * @param visible True to show, false to hide.
     */
    virtual void setVisible(bool visible) = 0;
};

} // namespace zappy

#endif // ZAPPY_IUICOMPONENT_HPP
