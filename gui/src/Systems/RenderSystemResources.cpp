/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** RenderSystem.cpp
*/

/**
 * @file RenderSystem.cpp
 * @brief Implementation of the RenderSystem class.
 */

#include "Color.hpp"
#include "Components/ComponentIncantationEffect.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "Components/ComponentTile.hpp"
#include "Components/FollowingEntity.hpp"
#include "Core.hpp"
#include "ECS/World.hpp"
#include "Graphics/TileTextures.hpp"
#include "Graphics/VoxelBatcher.hpp"
#include "Keyboard.hpp"
#include "Systems/RenderSystem.hpp"
#include "Vector3.hpp"
#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <ranges>
#include <raymath.h>
#include <rlgl.h>
#include <set>
#include <unordered_set>

#include "RenderSystemBatches.hpp"

namespace zappy {

void RenderSystem::_renderResources(World& w) {
    auto terrainStorage = w.get_storage<TerrainType>();
    if (!terrainStorage) {
        return;
    }

    raylib::Vector3 cameraTarget = _camera.target;
    auto& am = AssetManager::getInstance();

    std::vector<std::string> grassFood = {"food_steak", "food_carrot"};
    std::vector<std::string> mountainFood = {"food_ham_cooked", "food_cheese"};
    std::vector<std::string> waterFood = {"food_ham"};
    std::vector<std::string> sandFood = {"food_lettuce", "food_tomato"};
    std::vector<std::string> forestFood = {"food_ham"};
    std::vector<std::string> obsidianFood = {"food_ham_trash"};
    std::vector<std::string> luminousFood = {"food_steak", "food_lettuce"};
    std::vector<std::string> crystalFood = {"food_steak", "food_lettuce"};
    std::vector<std::string> magneticFood = {"food_ham"};

    for (auto const& [entity, type] : *terrainStorage) {
        auto pos = w.get_component<Position>(entity);
        auto inv = w.get_component<Inventory>(entity);
        if (pos && inv) {
            // Only render resources if tile is close to camera
            float dist = std::sqrt(std::pow(pos->x - cameraTarget.x, 2) +
                                   std::pow(pos->y - cameraTarget.z, 2));
            if (dist > 40.0f) {
                continue;
            }

            float yBase = 2.01f;

            // Use raw ingredient models for food, offset slightly around the tile
            if (inv->food > 0) {
                std::vector<std::string>* pool = nullptr;
                switch (type->current_type) {
                    case TerrainType::GRASS:
                        pool = &grassFood;
                        break;
                    case TerrainType::MOUNTAIN:
                        pool = &mountainFood;
                        break;
                    case TerrainType::WATER:
                        pool = &waterFood;
                        break;
                    case TerrainType::SAND:
                        pool = &sandFood;
                        break;
                    case TerrainType::FOREST:
                        pool = &forestFood;
                        break;
                    case TerrainType::OBSIDIAN_BARRENS:
                        pool = &obsidianFood;
                        break;
                    case TerrainType::LUMINOUS_ORCHARDS:
                        pool = &luminousFood;
                        break;
                    case TerrainType::CRYSTAL_CANYONS:
                        pool = &crystalFood;
                        break;
                    case TerrainType::MAGNETIC_TUNDRA:
                        pool = &magneticFood;
                        break;
                }

                int count = std::min(inv->food, 5); // Max 5 visual ingredients
                for (int i = 0; i < count; ++i) {
                    std::string selectedFood = "food_ham"; // fallback
                    if (pool && !pool->empty()) {
                        int index = (std::abs(pos->x * 137 + pos->y * 31 + i * 17)) % pool->size();
                        selectedFood = (*pool)[index];
                    }

                    raylib::Model& foodModel = am.getModel(selectedFood);
                    std::shared_ptr<BoundingBox> cBox = am.getBoundingBox(selectedFood, foodModel);

                    float sizeX = cBox->max.x - cBox->min.x;
                    float sizeZ = cBox->max.z - cBox->min.z;
                    float sizeY = cBox->max.y - cBox->min.y;
                    float maxDim = std::max({sizeX, sizeY, sizeZ});
                    float scale = (maxDim > 0) ? (0.25f / maxDim) : 0.10f; // Scale it nicely

                    raylib::Vector3 cCenter = {(cBox->min.x + cBox->max.x) / 2.0f, cBox->min.y,
                                               (cBox->min.z + cBox->max.z) / 2.0f};

                    // Deterministic random offset for food within the tile
                    float rX =
                        ((std::abs(pos->x * 71 + pos->y * 13 + i * 23)) % 81) / 100.0f - 0.4f;
                    float rZ =
                        ((std::abs(pos->x * 23 + pos->y * 89 + i * 47)) % 81) / 100.0f - 0.4f;

                    raylib::Vector3 cPos((float)pos->x + rX - (cCenter.x * scale),
                                         yBase - (cCenter.y * scale),
                                         (float)pos->y + rZ - (cCenter.z * scale));

                    float rotAngle = ((std::abs(pos->x * 47 + pos->y * 59 + i * 11)) % 360);

                    render::addInstance(selectedFood, cPos, {0, 1, 0}, rotAngle,
                                        {scale, scale, scale}, WHITE, foodModel.transform);
                }
            }

            auto drawResourceModel = [&](int count, float dx, float dz, raylib::Color tint) {
                if (count <= 0) {
                    return;
                }
                int modelIdx = std::clamp(count, 1, 9);
                std::string modelName = "resource_" + std::to_string(modelIdx);

                raylib::Model& rockModel = am.getModel(modelName);
                auto box = am.getBoundingBox(modelName, rockModel);
                float sizeX = box->max.x - box->min.x;
                float sizeZ = box->max.z - box->min.z;
                float sizeY = box->max.y - box->min.y;
                float maxDim = std::max({sizeX, sizeY, sizeZ});
                float rockScale = (maxDim > 0) ? (0.35f / maxDim) : 0.05f;

                raylib::Vector3 rockCenter = {(box->min.x + box->max.x) / 2.0f, box->min.y,
                                              (box->min.z + box->max.z) / 2.0f};

                raylib::Vector3 drawPos((float)pos->x - 0.3f + dx - (rockCenter.x * rockScale),
                                        yBase - (rockCenter.y * rockScale),
                                        (float)pos->y - 0.3f + dz - (rockCenter.z * rockScale));

                // Add deterministic rotation for variety
                float rotAngle =
                    ((std::abs(pos->x * 31 + pos->y * 73 + (int)(dx * 10) + (int)(dz * 10))) % 360);

                render::addInstance(modelName, drawPos, {0, 1, 0}, rotAngle,
                                    {rockScale, rockScale, rockScale}, tint, rockModel.transform);
            };

            drawResourceModel(inv->linemate, 0.0f, 0.0f, raylib::Color(110, 210, 120, 255));
            drawResourceModel(inv->deraumere, 0.2f, 0.0f, raylib::Color(100, 180, 240, 255));
            drawResourceModel(inv->sibur, 0.4f, 0.0f, raylib::Color(190, 130, 230, 255));
            drawResourceModel(inv->mendiane, 0.0f, 0.2f, raylib::Color(240, 220, 110, 255));
            drawResourceModel(inv->phiras, 0.2f, 0.2f, raylib::Color(235, 120, 120, 255));
            drawResourceModel(inv->thystame, 0.4f, 0.2f, raylib::Color(245, 245, 245, 255));
        }
    }
}

void RenderSystem::_renderAnimatedResources(World& w) {
    auto animatedStorage = w.get_storage<AnimatedResource>();
    if (!animatedStorage) {
        return;
    }
    auto& am = AssetManager::getInstance();

    for (auto const& [entity, animRes] : *animatedStorage) {
        auto move3D = w.get_component<MovementInterpolation3D>(entity);
        if (move3D) {
            std::string modelName = "food_ham";
            raylib::Color tint = WHITE;
            float scale = 0.25f;

            int resId = animRes->resourceId;
            if (resId == 0) { // Food
                modelName = "food_ham";
                tint = WHITE;
                scale = 0.25f;
            } else { // Resources 1-6
                modelName = "resource_1";
                scale = 0.35f;
                if (resId == 1) {
                    tint = raylib::Color(110, 210, 120, 255); // linemate
                } else if (resId == 2) {
                    tint = raylib::Color(100, 180, 240, 255); // deraumere
                } else if (resId == 3) {
                    tint = raylib::Color(190, 130, 230, 255); // sibur
                } else if (resId == 4) {
                    tint = raylib::Color(240, 220, 110, 255); // mendiane
                } else if (resId == 5) {
                    tint = raylib::Color(235, 120, 120, 255); // phiras
                } else if (resId == 6) {
                    tint = raylib::Color(245, 245, 245, 255); // thystame
                }
            }

            raylib::Model& model = am.getModel(modelName);
            auto box = am.getBoundingBox(modelName, model);
            float sizeX = box->max.x - box->min.x;
            float sizeZ = box->max.z - box->min.z;
            float sizeY = box->max.y - box->min.y;
            float maxDim = std::max({sizeX, sizeY, sizeZ});
            float finalScale = (maxDim > 0) ? (scale / maxDim) : 0.10f;

            raylib::Vector3 center = {(box->min.x + box->max.x) / 2.0f, box->min.y,
                                      (box->min.z + box->max.z) / 2.0f};

            raylib::Vector3 drawPos(move3D->visualX - (center.x * finalScale),
                                    move3D->visualZ - (center.y * finalScale),
                                    move3D->visualY - (center.z * finalScale));

            float rotAngle = static_cast<float>((entity.id() * 73) % 360);

            render::addInstance(modelName, drawPos, {0, 1, 0}, rotAngle,
                                {finalScale, finalScale, finalScale}, tint, model.transform);
        }
    }
}

} // namespace zappy
