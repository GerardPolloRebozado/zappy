/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AssetManager.cpp
*/

#include "BoundingBox.hpp"
#include "Graphics/AssetManager.hpp"
#include "Graphics/GraphicsErrors.hpp"
#include "Logging/Logger.hpp"
#include "Model.hpp"
#include "raylib.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

namespace zappy {

void AssetManager::_loadTextures() {
    try {
        _textures["mouse"] = std::make_unique<raylib::Texture2D>("assets/mouse.png");
        _textures["mouse_pressed"] =
            std::make_unique<raylib::Texture2D>("assets/mouse_pressed.png");

        // UI Textures
        _textures["menu_bg"] = std::make_unique<raylib::Texture2D>("assets/ui/menu_background.png");
        _textures["btn_normal"] =
            std::make_unique<raylib::Texture2D>("assets/ui/button_normal.png");
        _textures["btn_hover"] = std::make_unique<raylib::Texture2D>("assets/ui/button_active.png");
        _textures["btn_pressed"] =
            std::make_unique<raylib::Texture2D>("assets/ui/button_active.png");
        _textures["panel_bg"] =
            std::make_unique<raylib::Texture2D>("assets/ui/panel_background.png");

        _textures["hud_bg"] = std::make_unique<raylib::Texture2D>("assets/ui/hud_bg.png");
        _textures["hud_icon_food"] =
            std::make_unique<raylib::Texture2D>("assets/ui/hud_icon_food.png");
        _textures["hud_icon_level"] =
            std::make_unique<raylib::Texture2D>("assets/ui/hud_icon_level.png");
        _textures["hud_icon_team"] =
            std::make_unique<raylib::Texture2D>("assets/ui/hud_icon_team.png");
        _textures["hud_bar_bg"] = std::make_unique<raylib::Texture2D>("assets/ui/hud_bar_bg.png");
        _textures["hud_bar_fill"] =
            std::make_unique<raylib::Texture2D>("assets/ui/hud_bar_fill.png");
        _textures["hud_icon_resource"] =
            std::make_unique<raylib::Texture2D>("assets/ui/hud_icon_resource.png");
        _textures["input_bg"] =
            std::make_unique<raylib::Texture2D>("assets/ui/input_background.png");

        // Event icon textures
        _textures["evt_meteor_shower"] =
            std::make_unique<raylib::Texture2D>("assets/ui/meteor_shower.png");
        _textures["evt_solar_flare"] =
            std::make_unique<raylib::Texture2D>("assets/ui/solar_flare.png");
        _textures["evt_gravity_well"] =
            std::make_unique<raylib::Texture2D>("assets/ui/gravity_well.png");
        _textures["evt_psionic_echo"] =
            std::make_unique<raylib::Texture2D>("assets/ui/psionic_echo.png");

        _textures["mannequin"] =
            std::make_unique<raylib::Texture2D>("assets/models/inhabitant/mannequin_texture.png");

        _textures["laud"] = std::make_unique<raylib::Texture2D>("assets/ui/laud.png");
        _textures["play"] = std::make_unique<raylib::Texture2D>("assets/ui/play.png");
        _textures["stop"] = std::make_unique<raylib::Texture2D>("assets/ui/stop.png");
        _textures["cross"] = std::make_unique<raylib::Texture2D>("assets/ui/close.png");
        _textures["next"] = std::make_unique<raylib::Texture2D>("assets/ui/step_front.png");
        _textures["prev"] = std::make_unique<raylib::Texture2D>("assets/ui/step_back.png");

        // Apply custom tintable texture to all materials of the mannequin model
        if (_models.find("robot") != _models.end() &&
            _textures.find("mannequin") != _textures.end()) {
            for (int i = 0; i < _models["robot"]->materialCount; i++) {
                _models["robot"]->materials[i].maps[MATERIAL_MAP_ALBEDO].texture =
                    *_textures["mannequin"];
            }
        }

    } catch (const raylib::RaylibException& e) {
        log_error(ErrorAsset("Failed to load textures: " + std::string(e.what())).what());
    }
}

void AssetManager::_loadFonts() {
    try {
        _fonts["TextFont"] = std::make_unique<raylib::Font>("assets/fonts/BoldPixels.ttf");
        _fonts["HeaderFont"] = std::make_unique<raylib::Font>("assets/fonts/DungeonFont.ttf");
    } catch (const raylib::RaylibException& e) {
        log_error(ErrorAsset("Failed to load fonts: " + std::string(e.what())).what());
    }
}

} // namespace zappy
