/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AssetManager.cpp
*/

#include "Graphics/AssetManager.hpp"
#include <iostream>

namespace zappy {

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

void AssetManager::loadAll() {
    _loadModels();
    _loadTextures();
    _loadShaders();
}

void AssetManager::unloadAll() {
    _models.clear();
    _textures.clear();
    _shaders.clear();
    CloseWindow();
}

raylib::Model& AssetManager::getModel(const std::string& name) {
    if (_models.find(name) == _models.end()) {
        std::cerr << "AssetManager: Model " << name << " not found, returning empty model"
                  << std::endl;
        _models[name] = std::make_unique<raylib::Model>();
    }
    return *_models[name];
}

raylib::Texture2D& AssetManager::getTexture(const std::string& name) {
    if (_textures.find(name) == _textures.end()) {
        std::cerr << "AssetManager: Texture " << name << " not found, returning empty texture"
                  << std::endl;
        _textures[name] = std::make_unique<raylib::Texture2D>();
    }
    return *_textures[name];
}

raylib::Shader& AssetManager::getShader(const std::string& name) {
    if (_shaders.find(name) == _shaders.end()) {
        std::cerr << "AssetManager: Shader " << name << " not found, returning empty shader"
                  << std::endl;
        _shaders[name] = std::make_unique<raylib::Shader>();
    }
    return *_shaders[name];
}

// TODO: improve
void AssetManager::_loadModels() {
    try {
        _models["chicken"] = std::make_unique<raylib::Model>("assets/models/Chicken.obj");
        _models["robot"] = std::make_unique<raylib::Model>("assets/models/Robot1.obj");
    } catch (const raylib::RaylibException& e) {
        std::cerr << "AssetManager: Failed to load models: " << e.what() << std::endl;
    }
}

void AssetManager::_loadTextures() {
    try {
        _textures["mouse"] = std::make_unique<raylib::Texture2D>("assets/mouse.png");
        _textures["mouse_pressed"] =
            std::make_unique<raylib::Texture2D>("assets/mouse_pressed.png");
    } catch (const raylib::RaylibException& e) {
        std::cerr << "AssetManager: Failed to load textures: " << e.what() << std::endl;
    }
}

void AssetManager::_loadShaders() {
    // _shaders["ritual"] = std::make_unique<raylib::Shader>("assets/shaders/ritual.vs",
    // "assets/shaders/ritual.fs");
}

} // namespace zappy
