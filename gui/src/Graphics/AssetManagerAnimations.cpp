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

        if (_animations.count("inhabitant_general_Death_A")) {
            ::ModelAnimation* anim = _animations["inhabitant_general_Death_A"];
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

} // namespace zappy
