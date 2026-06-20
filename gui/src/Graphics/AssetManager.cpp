/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AssetManager.cpp
*/

#include "Graphics/AssetManager.hpp"
#include "BoundingBox.hpp"
#include "Graphics/GraphicsErrors.hpp"
#include "Logging/Logger.hpp"
#include "Model.hpp"
#include "raylib.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

namespace zappy {

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

void AssetManager::loadAll() {
    _loadModels();
    _loadTextures();
    _loadShaders();
    _loadFonts();
    _loadAnimations();
    _loadAudio();
}

void AssetManager::unloadAll() {
    for (auto& [name, animPair] : _animationArrays) {
        ::UnloadModelAnimations(animPair.first, animPair.second);
    }
    _animationArrays.clear();
    _animations.clear();

    _models.clear();
    _textures.clear();
    _shaders.clear();
    _fonts.clear();
    _boundingBoxes.clear();
    _sounds.clear();
    _musicPaths.clear();
}

raylib::Model& AssetManager::getModel(const std::string& name) {
    if (_models.find(name) == _models.end()) {
        log_error(ErrorAsset("Model " + name + " not found, returning empty model").what());
        _models[name] = std::make_unique<raylib::Model>();
    }
    return *_models[name];
}

raylib::Texture2D& AssetManager::getTexture(const std::string& name) {
    if (_textures.find(name) == _textures.end()) {
        log_error(ErrorAsset("Texture " + name + " not found, returning empty texture").what());
        _textures[name] = std::make_unique<raylib::Texture2D>();
    }
    return *_textures[name];
}

raylib::Font& AssetManager::getFont(const std::string& name) {
    if (_fonts.find(name) == _fonts.end()) {
        log_error(ErrorAsset("Font " + name + " not found, returning default font").what());
        _fonts[name] = std::make_unique<raylib::Font>(GetFontDefault());
    }
    return *_fonts[name];
}

raylib::Shader& AssetManager::getShader(const std::string& name) {
    if (_shaders.find(name) == _shaders.end()) {
        log_error(ErrorAsset("Shader " + name + " not found, returning empty shader").what());
        _shaders[name] = std::make_unique<raylib::Shader>();
    }
    return *_shaders[name];
}

raylib::Sound& AssetManager::getSound(const std::string& name) {
    if (_sounds.find(name) == _sounds.end()) {
        log_error(ErrorAsset("Sound " + name + " not found, returning empty sound").what());
        _sounds[name] = std::make_unique<raylib::Sound>();
    }
    return *_sounds[name];
}

std::string AssetManager::getMusicPath(const std::string& name) {
    if (_musicPaths.find(name) == _musicPaths.end()) {
        log_error(ErrorAsset("Music " + name + " not found, returning empty string").what());
        return "";
    }
    return _musicPaths[name];
}

std::shared_ptr<raylib::BoundingBox> AssetManager::getBoundingBox(const std::string& name,
                                                                  raylib::Model& model) {
    std::shared_ptr<raylib::BoundingBox> cached_bounding_box = _boundingBoxes[name];
    if (cached_bounding_box) {
        return cached_bounding_box;
    }

    std::cout << "Cache fail" << std::endl;
    BoundingBox new_bounding_box = model.GetBoundingBox();
    cached_bounding_box = std::make_shared<raylib::BoundingBox>(new_bounding_box);
    _boundingBoxes[name] = cached_bounding_box;

    return cached_bounding_box;
}

raylib::ModelAnimation& AssetManager::getAnimation(const std::string& name) {
    auto it = _animations.find(name);
    if (it != _animations.end()) {
        // We reinterpret_cast because raylib-cpp's ModelAnimation inherits from ::ModelAnimation
        return *reinterpret_cast<raylib::ModelAnimation*>(it->second);
    }
    throw ErrorAsset("Animation not found: " + name);
}

} // namespace zappy
