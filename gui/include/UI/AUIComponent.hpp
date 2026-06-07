/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AUIComponent.hpp
*/

#ifndef ZAPPY_AUICOMPONENT_HPP
#define ZAPPY_AUICOMPONENT_HPP

#include "IUIComponent.hpp"

namespace zappy {

class AUIComponent : public IUIComponent {
  public:
    AUIComponent(raylib::Rectangle bounds, int zIndex = 0);
    virtual ~AUIComponent() = default;

    virtual void update(float dt, raylib::Vector2 mousePos,
                        std::shared_ptr<std::vector<UIEvent>> events) override;
    virtual void render() override = 0;

    int getZIndex() const override;
    void setZIndex(int zIndex) override;

    raylib::Rectangle getBounds() const override;
    void setBounds(raylib::Rectangle bounds) override;

    bool isVisible() const override;
    void setVisible(bool visible) override;

  protected:
    raylib::Rectangle _bounds;
    int _zIndex;
    bool _isVisible;
};

} // namespace zappy

#endif // ZAPPY_AUICOMPONENT_HPP
