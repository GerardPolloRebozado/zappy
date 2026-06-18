/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AssetManager.hpp
*/

#ifndef ZAPPY_GUI_ASSETMANAGER_HPP
#define ZAPPY_GUI_ASSETMANAGER_HPP

#include "BoundingBox.hpp"
#include "raylib-cpp.hpp"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace zappy {

class AssetManager {
  public:
    static AssetManager& getInstance();

    void loadAll();
    void unloadAll();

    // Models
    raylib::Model& getModel(const std::string& name);

    // Fonts
    raylib::Font& getFont(const std::string& name);

    // Audio
    raylib::Sound& getSound(const std::string& name);
    std::string getMusicPath(const std::string& name);

    // Textures
    raylib::Texture2D& getTexture(const std::string& name);

    // Shaders
    raylib::Shader& getShader(const std::string& name);

    std::shared_ptr<raylib::BoundingBox> getBoundingBox(const std::string& name,
                                                        raylib::Model& model);

    /**
     * @brief Retrieves a loaded 3D model animation by its name identifier.
     * @param name The name of the animation to retrieve (e.g., "inhabitant_general_Idle_A").
     * @return Reference to the loaded raylib::ModelAnimation.
     * @throws ErrorAsset If the animation cannot be found.
     */
    raylib::ModelAnimation& getAnimation(const std::string& name);

  private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::map<std::string, std::unique_ptr<raylib::Model>> _models;
    std::map<std::string, std::unique_ptr<raylib::Texture2D>> _textures;
    std::map<std::string, std::unique_ptr<raylib::Shader>> _shaders;
    std::map<std::string, std::unique_ptr<raylib::Font>> _fonts;
    std::unordered_map<std::string, std::shared_ptr<raylib::BoundingBox>> _boundingBoxes;

    std::map<std::string, ::ModelAnimation*> _animations;
    std::map<std::string, std::pair<::ModelAnimation*, int>> _animationArrays;

    std::map<std::string, std::unique_ptr<raylib::Sound>> _sounds;
    std::map<std::string, std::string> _musicPaths;

    void _loadModels();
    void _loadTextures();
    void _loadShaders();
    void _loadFonts();
    void _loadAnimations();
    void _loadAudio();
};

} // namespace zappy

#endif // ZAPPY_GUI_ASSETMANAGER_HPP
