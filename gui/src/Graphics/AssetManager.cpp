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
}

void AssetManager::unloadAll() {
    _models.clear();
    _textures.clear();
    _shaders.clear();
    _fonts.clear();
    // CloseWindow();
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

// TODO: improve
void AssetManager::_loadModels() {
    try {
        _models["chicken"] = std::make_unique<raylib::Model>("assets/models/Chicken.obj");
        _models["robot"] = std::make_unique<raylib::Model>("assets/models/Robot1.obj");
        _models["tree_1"] = std::make_unique<raylib::Model>("assets/models/tree_1.obj");
        _models["tree_2"] = std::make_unique<raylib::Model>("assets/models/tree_2.obj");
        _models["tree_group_1"] = std::make_unique<raylib::Model>("assets/models/tree_group_1.obj");
        _models["tree_group_2"] = std::make_unique<raylib::Model>("assets/models/tree_group_2.obj");

        _models["rock1"] = std::make_unique<raylib::Model>("assets/models/rock1.obj");
        _models["rock2"] = std::make_unique<raylib::Model>("assets/models/rock2.obj");

        for (int i = 1; i <= 9; ++i) {
            std::string key = "resource_" + std::to_string(i);
            _models[key] = std::make_unique<raylib::Model>("assets/models/" + key + ".obj");
        }
        const std::vector<std::string> foodModels = {
            "food_ham",    "food_ham_cooked", "food_cheese",  "food_steak",
            "food_carrot", "food_ham_trash",  "food_lettuce", "food_tomato"};
        for (const auto& name : foodModels) {
            _models[name] = std::make_unique<raylib::Model>("assets/models/" + name + ".obj");
        }
        _models["egg"] = std::make_unique<raylib::Model>("assets/models/egg.obj");

        const std::vector<std::string> animals = {
            "Axolotl", "Bear",     "Bunny",   "Cat",    "Chicken", "Cow",    "Crocodile",
            "Dog",     "Elephant", "Fox",     "Frog",   "Mole",    "Monkey", "Mouse",
            "Panda",   "Parrot",   "Penguin", "Piglet", "Turtle",  "Unicorn"};
        for (const auto& animal : animals) {
            std::string lowerName = animal;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::string path = "assets/models/animals/" + animal + "/" + lowerName + ".vox.obj";
            _models["voxel_" + lowerName] = std::make_unique<raylib::Model>(path);
        }

    } catch (const raylib::RaylibException& e) {
        log_error(ErrorAsset("Failed to load models: " + std::string(e.what())).what());
    }
}

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

    } catch (const raylib::RaylibException& e) {
        log_error(ErrorAsset("Failed to load textures: " + std::string(e.what())).what());
    }
}

void AssetManager::_loadShaders() {
    // explanations of... stuff:
    // Pipeline: The sequence of steps the GPU takes to convert 3D data into 2D pixels

    // VS (Vertex Shader): The first programmable stage of the pipeline. It executes once per
    // vertex. Its primary role is transforming 3D coordinates into 2D screen space (gl_Position).

    // FS (Fragment Shader): The final programmable stage. It executes once per rasterized pixel
    // (fragment). Its primary role is calculating the final RGBA color output (finalColor).

    // R"(...)": C++ Raw String Literal. Allows embedding multi-line GLSL code without escaping.
    // in / out: Data flow qualifiers. 'in' receives data from the previous pipeline stage (e.g. CPU

    // -> VS, or VS -> FS), and 'out' passes data to the next stage.

    // uniform: Global variables set by the CPU that remain constant for the entire draw call.

    // vec2/3/4: Mathematical vectors representing points, directions, or RGBA colors.
    // mat4: A 4x4 transformation matrix (used to translate, rotate, and scale geometry).

    // Hardware Instancing Shader
    // This shader handles rendering multiple instances of the same model in a single draw call.
    // The vertex shader (instancingVS) applies an instance-specific transformation matrix
    // (instanceTransform) to position each instance correctly in world space, bypassing the
    // CPU overhead of individual draw commands.
    const char* instancingVS = R"(#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in mat4 instanceTransform;

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vec4(1.0);
    gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
}
)";

    // The fragment shader (instancingFS) applies texture and color tinting per pixel
    // It also discards fragments with an alpha value < 0.1 to correctly render
    // textures with transparency cutouts (like leaves or decals) without writing
    // to the depth buffer and occluding geometry behind them
    const char* instancingFS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.1) discard; // Support alpha cutouts
    finalColor = texelColor * colDiffuse * fragColor;
}
)";

    _shaders["instancing"] =
        std::make_unique<raylib::Shader>(::LoadShaderFromMemory(instancingVS, instancingFS));

    // Alpha Cutout Shader
    // Used primarily for flat 2D terrain decals. It ensures fragments with low alpha
    // do not write to the depth buffer, preventing transparent boundary areas from
    // incorrectly hiding the underlying base terrain tiles
    const char* alphaFS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.1) discard;
    finalColor = texelColor * fragColor;
}
)";

    _shaders["alphaCutout"] =
        std::make_unique<raylib::Shader>(::LoadShaderFromMemory(nullptr, alphaFS));

    auto& shader = *_shaders["instancing"];
    for (auto& [name, model] : _models) {
        for (int i = 0; i < model->materialCount; i++) {
            model->materials[i].shader = shader;
        }
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
