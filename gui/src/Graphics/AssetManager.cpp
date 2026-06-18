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

// TODO: improve
void AssetManager::_loadModels() {
    try {
        _models["chicken"] = std::make_unique<raylib::Model>("assets/models/Chicken.obj");
        // Bypass raylib::Model strict constructor to avoid false positive IsModelValid exceptions
        // on glTF bone data
        ::Model rawModel = ::LoadModel("assets/models/inhabitant/mannequin.glb");

        // Strip out vertex colors and apply to all materials so tinting works properly
        for (int i = 0; i < rawModel.meshCount; i++) {
            if (rawModel.meshes[i].colors != nullptr) {
                for (int j = 0; j < rawModel.meshes[i].vertexCount * 4; j++) {
                    rawModel.meshes[i].colors[j] = 255;
                }
                ::UpdateMeshBuffer(rawModel.meshes[i], 3, rawModel.meshes[i].colors,
                                   rawModel.meshes[i].vertexCount * 4 * sizeof(unsigned char), 0);
            }
        }

        _models["robot"] = std::make_unique<raylib::Model>(rawModel);
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

    const char* incantationFS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform float time;

out vec4 finalColor;

void main()
{
    vec2 p = fragTexCoord - 0.5;
    float r = length(p);
    float a = atan(p.y, p.x);
    // Smoothness factor for anti-aliasing
    float sm = 0.01;
    // Outer boundary (don't draw outside the quad)
    if (r > 0.5) discard;
    // Rotation
    float a1 = a + time * 0.5; // Outer rotates one way
    float a2 = a - time * 0.3; // Inner rotates another
    float intensity = 0.0;
    // Outer thick ring
    intensity += smoothstep(0.02, 0.0, abs(r - 0.45));
    // Outer dashed/runic ring
    float dashRing = smoothstep(0.03, 0.0, abs(r - 0.4));
    float dashes = step(0.0, sin(a1 * 30.0));
    intensity += dashRing * dashes;

    // Inner thin ring
    intensity += smoothstep(0.01, 0.0, abs(r - 0.32));

    // Hexagram (Two overlapping triangles)
    // Distance to a triangle from center
    float tri1 = max(max(r * cos(a2), r * cos(a2 + 2.09439)), r * cos(a2 - 2.09439));
    float tri2 = max(max(r * cos(a2 + 3.14159), r * cos(a2 + 3.14159 + 2.09439)), r * cos(a2 + 3.14159 - 2.09439));
    intensity += smoothstep(0.01, 0.0, abs(tri1 - 0.15));
    intensity += smoothstep(0.01, 0.0, abs(tri2 - 0.15));
    // Core pulsing dot
    float pulse = sin(time * 4.0) * 0.5 + 0.5;
    intensity += smoothstep(0.05 + pulse * 0.02, 0.0, r);

    // Magic fire colors
    vec3 primaryColor = vec3(1.0, 1.0, 1.0); // Bright white
    vec3 secondaryColor = vec3(0.8, 0.9, 1.0); // Light heavenly blue
    vec3 tertiaryColor = vec3(0.5, 0.7, 1.0); // Deep heavenly blue
    float coreGlow = smoothstep(0.4, 0.0, r) * 0.5 * pulse;
    vec3 finalRGB = mix(tertiaryColor, primaryColor, intensity) * (intensity + coreGlow);
    float finalAlpha = (intensity + coreGlow) * fragColor.a;

    finalColor = vec4(finalRGB, finalAlpha);
    if (finalColor.a < 0.05) discard;
}
)";

    _shaders["incantation_aura"] =
        std::make_unique<raylib::Shader>(::LoadShaderFromMemory(nullptr, incantationFS));

    auto& shader = *_shaders["instancing"];
    for (auto& [name, model] : _models) {
        if (name == "robot") {
            continue;
        }
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

raylib::ModelAnimation& AssetManager::getAnimation(const std::string& name) {
    auto it = _animations.find(name);
    if (it != _animations.end()) {
        // We reinterpret_cast because raylib-cpp's ModelAnimation inherits from ::ModelAnimation
        return *reinterpret_cast<raylib::ModelAnimation*>(it->second);
    }
    throw ErrorAsset("Animation not found: " + name);
}

void AssetManager::_loadAnimations() {
    try {
        int count = 0;
        ::ModelAnimation* generalAnims =
            ::LoadModelAnimations("assets/models/inhabitant/anim_general.glb", &count);
        _animationArrays["inhabitant_general"] = {generalAnims, count};
        for (int i = 0; i < count; i++) {
            _animations["inhabitant_general_" + std::string(generalAnims[i].name)] =
                &generalAnims[i];
        }

        int countMove = 0;
        ::ModelAnimation* moveAnims =
            ::LoadModelAnimations("assets/models/inhabitant/anim_movement.glb", &countMove);
        _animationArrays["inhabitant_movement"] = {moveAnims, countMove};
        for (int i = 0; i < countMove; i++) {
            _animations["inhabitant_movement_" + std::string(moveAnims[i].name)] = &moveAnims[i];
        }

        int countSim = 0;
        ::ModelAnimation* simAnims =
            ::LoadModelAnimations("assets/models/inhabitant/anim_simulation.glb", &countSim);
        _animationArrays["inhabitant_simulation"] = {simAnims, countSim};
        for (int i = 0; i < countSim; i++) {
            _animations["inhabitant_simulation_" + std::string(simAnims[i].name)] = &simAnims[i];
        }

        int countSpecial = 0;
        ::ModelAnimation* specialAnims =
            ::LoadModelAnimations("assets/models/inhabitant/anim_special.glb", &countSpecial);
        _animationArrays["inhabitant_special"] = {specialAnims, countSpecial};
        for (int i = 0; i < countSpecial; i++) {
            _animations["inhabitant_special_" + std::string(specialAnims[i].name)] =
                &specialAnims[i];
        }

        // Apply mannequin bone mapping hack immediately
        int animMapping[] = {0,  1, 2, 3,  4,  5,  6,  7,  14, 15, 16,
                             17, 8, 9, 10, 11, 12, 19, 20, 21, 22};

        if (_animations.count("inhabitant_general_Idle_A")) {
            ::ModelAnimation* anim = _animations["inhabitant_general_Idle_A"];
            for (int f = 0; f < anim->keyframeCount; f++) {
                std::vector<Transform> oldPose(anim->keyframePoses[f],
                                               anim->keyframePoses[f] + anim->boneCount);
                for (int i = 0; i < 21; i++) {
                    anim->keyframePoses[f][i] = oldPose[animMapping[i]];
                }
            }
            anim->boneCount = 21;
        }

        if (_animations.count("inhabitant_movement_Walking_A")) {
            ::ModelAnimation* anim = _animations["inhabitant_movement_Walking_A"];
            for (int f = 0; f < anim->keyframeCount; f++) {
                std::vector<Transform> oldPose(anim->keyframePoses[f],
                                               anim->keyframePoses[f] + anim->boneCount);
                for (int i = 0; i < 21; i++) {
                    anim->keyframePoses[f][i] = oldPose[animMapping[i]];
                }
            }
            anim->boneCount = 21;
        }

        if (_animations.count("inhabitant_simulation_Push_Ups")) {
            int simMapping[] = {0,  1,  19, 20, 21, 22, 6, 7, 13, 14, 15,
                                16, 18, 8,  9,  10, 11, 2, 3, 4,  5};
            ::ModelAnimation* anim = _animations["inhabitant_simulation_Push_Ups"];
            for (int f = 0; f < anim->keyframeCount; f++) {
                std::vector<Transform> oldPose(anim->keyframePoses[f],
                                               anim->keyframePoses[f] + anim->boneCount);
                for (int i = 0; i < 21; i++) {
                    anim->keyframePoses[f][i] = oldPose[simMapping[i]];
                }
            }
            anim->boneCount = 21;
        }

        if (_animations.count("inhabitant_special_Skeletons_Awaken_Standing")) {
            ::ModelAnimation* anim = _animations["inhabitant_special_Skeletons_Awaken_Standing"];
            for (int f = 0; f < anim->keyframeCount; f++) {
                std::vector<Transform> oldPose(anim->keyframePoses[f],
                                               anim->keyframePoses[f] + anim->boneCount);
                for (int i = 0; i < 21; i++) {
                    anim->keyframePoses[f][i] = oldPose[animMapping[i]];
                }
            }
            anim->boneCount = 21;
        }
    } catch (const raylib::RaylibException& e) {
        log_error(ErrorAsset("Failed to load animations: " + std::string(e.what())).what());
    }
}

void AssetManager::_loadAudio() {
    try {
        _sounds["egg_layed"] =
            std::make_unique<raylib::Sound>("assets/sounds/effect/egg_layed.mp3");
        _musicPaths["country"] = "assets/sounds/music/country.mp3";
    } catch (const raylib::RaylibException& e) {
        log_error(ErrorAsset("Failed to load audio: " + std::string(e.what())).what());
    }
}

} // namespace zappy
