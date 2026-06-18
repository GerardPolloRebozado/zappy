#ifndef ZAPPY_UICHATPANEL_HPP
#define ZAPPY_UICHATPANEL_HPP

#include "AUIComponent.hpp"
#include "ECS/World.hpp"
#include "Graphics/AssetManager.hpp"
#include "Logging/ChatLogs.hpp"
#include <map>
#include <memory>
#include <string>

namespace zappy {

class UIChatPanel : public AUIComponent {
  public:
    UIChatPanel(raylib::Rectangle bounds, std::shared_ptr<ChatLogs> chatLogs, World& world,
                int zIndex = 0);
    ~UIChatPanel() override = default;

    void update(float dt, raylib::Vector2 mousePos,
                std::shared_ptr<std::vector<UIEvent>> events) override;
    void render() override;

  private:
    std::shared_ptr<ChatLogs> _chatLogs;
    World& _world;
    int _fontSize;
    float _spacing;
    std::map<std::string, raylib::Color> _typeColors = {
        {"ERROR", raylib::Color::Red()},        {"DEATH", raylib::Color::Red()},
        {"JOIN", raylib::Color::Green()},       {"CHAT", raylib::Color::Yellow()},
        {"BROADCAST", raylib::Color::Yellow()},
    };
};

} // namespace zappy

#endif // ZAPPY_UICHATPANEL_HPP