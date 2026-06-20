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

void AssetManager::_loadAudio() {
    try {
        _sounds["egg_layed"] =
            std::make_unique<raylib::Sound>("assets/sounds/effect/egg_layed.mp3");
        _sounds["incantation_end"] =
            std::make_unique<raylib::Sound>("assets/sounds/effect/incantation_end.mp3");
        _sounds["incantation_end"]->SetVolume(5.0f);
        _sounds["death"] = std::make_unique<raylib::Sound>("assets/sounds/effect/death.mp3");
        _musicPaths["country"] = "assets/sounds/music/country.mp3";
    } catch (const raylib::RaylibException& e) {
        log_error(ErrorAsset("Failed to load audio: " + std::string(e.what())).what());
    }
}

} // namespace zappy
