/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** UIEventMenu.cpp
*/

#include "UI/UIEventMenu.hpp"
#include "Components/ComponentShared.hpp"
#include "Graphics/AssetManager.hpp"
#include "NetworkManager.hpp"

namespace zappy {

const std::array<UIEventMenu::EventDef, 4> UIEventMenu::EVENT_DEFS = {{
    {"meteor_shower", "evt_meteor_shower", "Meteor Shower"},
    {"solar_flare", "evt_solar_flare", "Solar Flare"},
    {"gravity_well", "evt_gravity_well", "Gravity Well"},
    {"psionic_echo", "evt_psionic_echo", "Psionic Echo"},
}};

static constexpr float ICON_SIZE = 72.0f;
static constexpr float ICON_PAD = 14.0f;
static constexpr float PANEL_PAD = 16.0f;
static constexpr float FRAME_BORDER = 3.0f;
static constexpr float LABEL_GAP = 8.0f;
static constexpr float LABEL_H = 18.0f;
static constexpr float SLIDER_GAP = 12.0f;
static constexpr float SLIDER_H = 38.0f;
static constexpr int LABEL_FONT_SIZE = 9;

/**
 * @brief Builds the toggle button, popup layout, event icon row, and frequency slider.
 */
UIEventMenu::UIEventMenu(raylib::Rectangle bounds, World& world, NetworkManager& network,
                         std::shared_ptr<ChatLogs> chatLogs, int zIndex)
    : AUIComponent(bounds, nullptr, zIndex), _world(world), _network(network),
      _chatLogs(std::move(chatLogs)), _isOpen(false) {

    _toggleButton = std::make_unique<UIButton>(
        bounds, "Events", [this]() { _isOpen = !_isOpen; }, "btn_normal", "btn_hover",
        "btn_pressed", zIndex + 1);

    float gridW = ICON_SIZE * 4.0f + ICON_PAD * 3.0f;
    float iconBlockH = ICON_SIZE + FRAME_BORDER * 2.0f;
    float popupW = gridW + PANEL_PAD * 2.0f;
    float popupH = PANEL_PAD + iconBlockH + LABEL_GAP + LABEL_H + SLIDER_GAP + SLIDER_H + PANEL_PAD;

    float popupX = bounds.x + bounds.width - popupW;
    float popupY = bounds.y - popupH - 5.0f;
    _popupBounds = raylib::Rectangle(popupX, popupY, popupW, popupH);

    float gridStartX = popupX + PANEL_PAD;
    float gridStartY = popupY + PANEL_PAD;

    for (size_t i = 0; i < 4; ++i) {
        float bx = gridStartX + static_cast<float>(i) * (ICON_SIZE + ICON_PAD);
        float by = gridStartY;

        std::string evName = EVENT_DEFS[i].name;
        _eventButtons[i] = std::make_unique<UIButton>(
            raylib::Rectangle{bx, by, ICON_SIZE, ICON_SIZE}, "",
            [this, evName]() { _tryTriggerEvent(evName); }, EVENT_DEFS[i].texKey,
            EVENT_DEFS[i].texKey, EVENT_DEFS[i].texKey, zIndex + 2);
    }

    float sliderX = gridStartX;
    float sliderY = gridStartY + iconBlockH + LABEL_GAP + LABEL_H + SLIDER_GAP;
    float sliderW = gridW;
    _slider = std::make_unique<UISlider>(raylib::Rectangle{sliderX, sliderY, sliderW, SLIDER_H},
                                         _world, _network, zIndex + 2);
}

/**
 * @brief Attempts to trigger a map event if none is currently active.
 * @param eventName Protocol event identifier (e.g. "solar_flare").
 *
 * Reads the cached MapEvent ECS component. If an event is already running,
 * posts a warning to the chat log instead of sending mev.
 */
void UIEventMenu::_tryTriggerEvent(const std::string& eventName) {
    auto storage = _world.get_storage<MapEvent>();
    if (storage) {
        for (auto const& [entity, mapEvent] : *storage) {
            if (mapEvent->active) {
                if (_chatLogs) {
                    _chatLogs->addChatLog("Cannot trigger " + eventName + ": " + mapEvent->name +
                                              " is active",
                                          "EVENT");
                }
                return;
            }
            break;
        }
    }
    _network.triggerMapEvent(eventName);
}

/**
 * @brief Draws a Minecraft-style 9-patch panel background.
 * @param rect Target screen rectangle for the popup panel.
 */
void UIEventMenu::_drawNPatchBackground(raylib::Rectangle rect) {
    auto& tex = AssetManager::getInstance().getTexture("menu_bg");
    if (tex.id != 0) {
        ::NPatchInfo npatchInfo = {
            (raylib::Rectangle){0.0f, 0.0f, (float)tex.width, (float)tex.height},
            20,
            20,
            20,
            20,
            NPATCH_NINE_PATCH};
        tex.Draw(npatchInfo, rect, raylib::Vector2(0, 0), 0.0f, raylib::Color::White());
    } else {
        rect.Draw(raylib::Color(15, 20, 40, 230));
        rect.DrawLines(raylib::Color(0, 100, 255, 255), 2.0f);
    }
}

/**
 * @brief Updates the toggle button and, when open, the event buttons and slider.
 */
void UIEventMenu::update(float dt, raylib::Vector2 mousePos,
                         std::shared_ptr<std::vector<UIEvent>> events) {
    _toggleButton->update(dt, mousePos, events);

    if (!_isOpen) {
        return;
    }

    _slider->update(dt, mousePos, events);

    for (auto& btn : _eventButtons) {
        btn->update(dt, mousePos, events);
    }
}

/**
 * @brief Renders the toggle button and, when open, the popup panel contents.
 *
 * Draws the 9-patch background, event icon frames, labels, and frequency slider.
 */
void UIEventMenu::render() {
    _toggleButton->render();

    if (!_isOpen) {
        return;
    }

    _drawNPatchBackground(_popupBounds);

    for (size_t i = 0; i < 4; ++i) {
        raylib::Rectangle iconBounds = _eventButtons[i]->getBounds();

        raylib::Rectangle frameBounds = {iconBounds.x - FRAME_BORDER, iconBounds.y - FRAME_BORDER,
                                         iconBounds.width + FRAME_BORDER * 2.0f,
                                         iconBounds.height + FRAME_BORDER * 2.0f};
        frameBounds.Draw(raylib::Color(40, 30, 20, 200));
        frameBounds.DrawLines(raylib::Color(80, 60, 40, 255), 2.0f);

        _eventButtons[i]->render();

        float labelY = frameBounds.y + frameBounds.height + LABEL_GAP;
        const std::string& label = EVENT_DEFS[i].label;
        raylib::Text labelText(label, LABEL_FONT_SIZE, raylib::Color::RayWhite(),
                               AssetManager::getInstance().getFont("TextFont"), 1.0f);
        raylib::Vector2 textSize = labelText.MeasureEx();
        float labelX = iconBounds.x + (iconBounds.width - textSize.x) / 2.0f;
        labelText.Draw(labelX, labelY);
    }

    _slider->render();
}

} // namespace zappy
