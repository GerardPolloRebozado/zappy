/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** UiRadio.cpp
*/
#ifndef ZAPPY_UIRADIO_HPP
#define ZAPPY_UIRADIO_HPP

#include "AUIComponent.hpp"
#include "ECS/World.hpp"
#include "Graphics/AssetManager.hpp"
#include "UI/UIButton.hpp"
#include "UIPanel.hpp"
#include <memory>
#include <string>
#include <vector>

namespace zappy {
class UIRadio : public AUIComponent {
  public:
    UIRadio(raylib::Rectangle bounds, World& world, int zIndex = 0, int songIdx = 0,
            std::vector<std::string> playlist = {});
    ~UIRadio() override = default;

    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;
    void render() override;

    int getZIndex() const override;
    void setZIndex(int zIndex) override;

    raylib::Rectangle getBounds() const override;
    void setBounds(raylib::Rectangle bounds) override;

    bool isVisible() const override;
    void setVisible(bool visible) override;

  private:
    bool opened = false;
    uint _songIdxM;
    std::vector<std::string> _playlist;

    World& _world;

    std::shared_ptr<UIPanel> _menuRadio;
    std::shared_ptr<UIButton> _openRadio;
    std::shared_ptr<UIButton> _closeRadio;

    std::shared_ptr<UIButton> _prevSong;
    std::shared_ptr<UIButton> _playBtn;
    std::shared_ptr<UIButton> _stopBtn;
    std::shared_ptr<UIButton> _nextSong;
};
} // namespace zappy

#endif // ZAPPY_UIRADIO_HPP