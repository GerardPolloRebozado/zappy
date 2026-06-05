/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AssetManager.hpp
*/

#ifndef ZAPPY_GUI_ASSETMANAGER_HPP
#define ZAPPY_GUI_ASSETMANAGER_HPP

#include "raylib-cpp.hpp"
#include <map>
#include <memory>
#include <string>

namespace zappy {

class AssetManager {
  public:
    static AssetManager& getInstance();

    void loadAll();

    // Models
    raylib::Model& getModel(const std::string& name);

    // Textures
    raylib::Texture2D& getTexture(const std::string& name);

    // Shaders
    raylib::Shader& getShader(const std::string& name);

  private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::map<std::string, std::unique_ptr<raylib::Model>> _models;
    std::map<std::string, std::unique_ptr<raylib::Texture2D>> _textures;
    std::map<std::string, std::unique_ptr<raylib::Shader>> _shaders;

    void _loadModels();
    void _loadTextures();
    void _loadShaders();
};

} // namespace zappy

#endif // ZAPPY_GUI_ASSETMANAGER_HPP
