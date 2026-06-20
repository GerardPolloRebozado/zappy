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
        _textures["input_bg"] =
            std::make_unique<raylib::Texture2D>("assets/ui/input_background.png");

        _textures["mannequin"] =
            std::make_unique<raylib::Texture2D>("assets/models/inhabitant/mannequin_texture.png");

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
