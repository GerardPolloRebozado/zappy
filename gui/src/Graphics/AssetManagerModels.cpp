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

        _models["skull"] = std::make_unique<raylib::Model>("assets/models/tomb/skull_candle.obj");

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

} // namespace zappy
