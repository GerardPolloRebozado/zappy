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

#include "Systems/RenderSystemBatches.hpp"

namespace zappy {

void RenderSystem::_renderTerrain(World& w) {
    auto terrainStorage = w.get_storage<TerrainType>();
    if (!terrainStorage) {
        return;
    }

    auto hashPos = [](int x, int z) -> uint64_t {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(z);
    };

    std::unordered_set<uint64_t> inhabitantPositions;
    auto orientationStorage = w.get_storage<Orientation>();
    if (orientationStorage) {
        for (auto const& [entity, orientationPtr] : *orientationStorage) {
            auto ppos = w.get_component<Position>(entity);
            if (ppos) {
                inhabitantPositions.insert(hashPos(ppos->x, ppos->y));
            }
        }
    }

    std::unordered_map<uint64_t, TerrainType::Type> mapGrid;
    for (auto const& [entity, type] : *terrainStorage) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            mapGrid[hashPos(pos->x, pos->y)] = type->current_type;
        }
    }

    raylib::Vector3 cameraTarget = _camera.target;
    raylib::Vector3 cameraPos = _camera.position;

    // Calculate dynamic cull distance based on zoom level
    float camZoomDist = std::sqrt(std::pow(cameraPos.x - cameraTarget.x, 2) +
                                  std::pow(cameraPos.y - cameraTarget.y, 2) +
                                  std::pow(cameraPos.z - cameraTarget.z, 2));
    float cullRadius = camZoomDist * 1.5f + 30.0f; // Scale visibility with zoom
    float cullDistSq = cullRadius * cullRadius;

    // Flatten camera vectors to XZ plane for 2D frustum culling
    raylib::Vector3 forward = {cameraTarget.x - cameraPos.x, 0.0f, cameraTarget.z - cameraPos.z};
    float fLen = std::sqrt(forward.x * forward.x + forward.z * forward.z);
    if (fLen > 0.0f) {
        forward.x /= fLen;
        forward.z /= fLen;
    }

    static Tiletextures textures;
    VoxelBatcher batcher;

    std::shared_ptr<raylib::Texture2D> atlas = textures.getAtlas();

    struct TopFaceData {
        raylib::Vector3 pos;
        float u, v, uw, vh;
        raylib::Color color;
    };
    std::vector<TopFaceData> topFaceBatches;
    std::vector<TopFaceData> decalBatches;
    std::vector<raylib::Vector3> sideFaceBatches;

    auto getMapType = [&](int nx, int ny) -> TerrainType::Type {
        uint64_t key = hashPos(nx, ny);
        if (mapGrid.count(key)) {
            return mapGrid.at(key);
        }
        return TerrainType::GRASS; // Fallback, shouldn't bleed since priority is low
    };

    for (auto const& [entity, type] : *terrainStorage) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            // Dynamic distance culling
            float dx = pos->x - cameraTarget.x;
            float dz = pos->y - cameraTarget.z;
            if (dx * dx + dz * dz > cullDistSq) {
                continue;
            }

            // Frustum Culling: cull tiles that are well behind the camera
            float cx = pos->x - cameraPos.x;
            float cz = pos->y - cameraPos.z;
            float dot = (cx * forward.x + cz * forward.z);
            if (dot < -(camZoomDist * 0.8f + 10.0f)) { // Buffer heavily scales with zoom
                continue;
            }

            const raylib::Vector3 vpos(static_cast<float>(pos->x), 1.5f,
                                       static_cast<float>(pos->y));

            raylib::Vector3 sidePos = vpos;
            sidePos.y -= 0.02f; // Shift down slightly
            sideFaceBatches.push_back(sidePos);

            raylib::Vector3 planePos = vpos;
            planePos.y += 0.5f; // Plane rests exactly on top of the original cube height

            int variation = (pos->x * 73856093 ^ pos->y * 19349663) % 4;
            if (variation < 0) {
                variation += 4;
            }

            float u, v, uw, vh;
            textures.getTileUVs(type->current_type, variation, 0, u, v, uw, vh); // Col 0 is Base
            raylib::Color tileColor = (type->current_type == TerrainType::GRASS)
                                          ? raylib::Color{210, 210, 210, 255}
                                          : WHITE;
            topFaceBatches.push_back({planePos, u, v, uw, vh, tileColor});

            // Now calculate decals
            int currentPriority = textures.getPriority(type->current_type);

            auto checkDecal = [&](int nx, int ny, DecalDirection col) {
                uint64_t key = hashPos(nx, ny);
                if (!mapGrid.count(key)) {
                    return;
                }
                TerrainType::Type nType = mapGrid.at(key);
                int nPriority = textures.getPriority(nType);
                if (nPriority > currentPriority) {
                    float du, dv, duw, dvh;
                    textures.getTileUVs(nType, variation, static_cast<int>(col), du, dv, duw, dvh);
                    raylib::Vector3 decalPos = planePos;
                    raylib::Color decalColor =
                        (nType == TerrainType::GRASS) ? raylib::Color{210, 210, 210, 255} : WHITE;
                    decalBatches.push_back({decalPos, du, dv, duw, dvh, decalColor});
                }
            };

            checkDecal(pos->x, pos->y - 1, DecalDirection::NORTH); // North
            checkDecal(pos->x, pos->y + 1, DecalDirection::SOUTH); // South
            checkDecal(pos->x + 1, pos->y, DecalDirection::EAST);  // East
            checkDecal(pos->x - 1, pos->y, DecalDirection::WEST);  // West

            checkDecal(pos->x - 1, pos->y - 1, DecalDirection::NORTH_WEST); // NW
            checkDecal(pos->x + 1, pos->y - 1, DecalDirection::NORTH_EAST); // NE
            checkDecal(pos->x - 1, pos->y + 1, DecalDirection::SOUTH_WEST); // SW
            checkDecal(pos->x + 1, pos->y + 1, DecalDirection::SOUTH_EAST); // SE

            if (type->current_type == TerrainType::FOREST ||
                type->current_type == TerrainType::MAGNETIC_TUNDRA) {
                std::string treeNames[] = {"tree_1", "tree_2", "tree_group_1", "tree_group_2"};
                std::string alienTreeNames[] = {"alien_tree_1", "alien_tree_2",
                                                "alien_tree_group_1", "alien_tree_group_2"};
                int treeIdx = (pos->x * 12345 + pos->y * 67890) % 4;
                if (treeIdx < 0) {
                    treeIdx += 4;
                }
                std::string treeKey = (type->current_type == TerrainType::FOREST)
                                          ? treeNames[treeIdx]
                                          : alienTreeNames[treeIdx];

                raylib::Model& treeModel = AssetManager::getInstance().getModel(treeKey);
                auto& am = AssetManager::getInstance();
                std::shared_ptr<BoundingBox> box = am.getBoundingBox(treeKey, treeModel);

                float sizeX = box->max.x - box->min.x;
                float sizeZ = box->max.z - box->min.z;

                if (sizeX > 0 && sizeZ > 0) {
                    bool playerOnTile = inhabitantPositions.count(hashPos(pos->x, pos->y)) > 0;

                    if (!playerOnTile) {
                        float scale =
                            0.85f; // Constant scale keeps all trees proportional to each other

                        raylib::Vector3 centerOffset((box->max.x + box->min.x) / 2.0f * scale,
                                                     box->min.y * scale,
                                                     (box->max.z + box->min.z) / 2.0f * scale);

                        raylib::Vector3 drawPos(vpos.x - centerOffset.x, 2.0f - centerOffset.y,
                                                vpos.z - centerOffset.z);

                        // Deterministic random offset within the tile
                        float rX = ((std::abs(pos->x * 137 + pos->y * 31)) % 41) / 100.0f - 0.2f;
                        float rZ = ((std::abs(pos->x * 19 + pos->y * 101)) % 41) / 100.0f - 0.2f;
                        drawPos.x += rX;
                        drawPos.z += rZ;

                        render::addInstance(treeKey, drawPos, {0, 1, 0}, 0.0f,
                                            {scale, scale, scale}, WHITE, treeModel.transform);
                    }
                }
            }

            if (type->current_type == TerrainType::GRASS) {
                bool playerOnTile = inhabitantPositions.count(hashPos(pos->x, pos->y)) > 0;
                bool hasBush = ((pos->x * 314159 + pos->y * 271828) % 7) == 0;

                std::string grassNames[] = {"grass_item_1", "grass_item_2"};
                std::string bushNames[] = {"bush_1", "bush_2"};

                int grassIdx = (pos->x * 777 + pos->y * 888) % 2;
                if (grassIdx < 0) {
                    grassIdx += 2;
                }
                int bushIdx = (pos->x * 999 + pos->y * 111) % 2;
                if (bushIdx < 0) {
                    bushIdx += 2;
                }

                auto renderProp = [&](const std::string& key, float scale, float offsetY, float rX,
                                      float rZ, float rotationAngle) {
                    raylib::Model& model = AssetManager::getInstance().getModel(key);
                    std::shared_ptr<BoundingBox> box =
                        AssetManager::getInstance().getBoundingBox(key, model);

                    float sizeX = box->max.x - box->min.x;
                    if (sizeX <= 0) {
                        return;
                    }

                    raylib::Vector3 centerOffset((box->max.x + box->min.x) / 2.0f * scale,
                                                 box->min.y * scale,
                                                 (box->max.z + box->min.z) / 2.0f * scale);
                    raylib::Vector3 drawPos(vpos.x - centerOffset.x + rX,
                                            2.0f - centerOffset.y + offsetY,
                                            vpos.z - centerOffset.z + rZ);
                    render::addInstance(key, drawPos, {0, 1, 0}, rotationAngle,
                                        {scale, scale, scale}, WHITE, model.transform);
                };

                // Deterministic random offset within the tile for grass
                float rXG = ((std::abs(pos->x * 137 + pos->y * 31)) % 41) / 100.0f - 0.2f;
                float rZG = ((std::abs(pos->x * 19 + pos->y * 101)) % 41) / 100.0f - 0.2f;
                float rotG = static_cast<float>((std::abs(pos->x * 9283 + pos->y * 1234)) % 360);

                // Draw grass on all
                renderProp(grassNames[grassIdx], 0.65f, 0.0f, rXG, rZG, rotG);

                if (hasBush && !playerOnTile) {
                    // Different offset for bush
                    float rXB = ((std::abs(pos->x * 83 + pos->y * 47)) % 41) / 100.0f - 0.2f;
                    float rZB = ((std::abs(pos->x * 71 + pos->y * 59)) % 41) / 100.0f - 0.2f;
                    float rotB =
                        static_cast<float>((std::abs(pos->x * 5678 + pos->y * 4321)) % 360);
                    renderProp(bushNames[bushIdx], 0.65f, 0.0f, rXB, rZB, rotB);
                }
            }

            if (pos->x == _hoveredX && pos->y == _hoveredZ) {
                _renderHoverEffect(pos->x, pos->y);
            }
        }
    }

    // Render batches
    batcher.beginSolidBatch();
    for (const auto& sidePos : sideFaceBatches) {
        batcher.addSideFaces(sidePos, 1.0f, 0.96f, 1.0f, raylib::Color(110, 110, 110, 255), true,
                             true, true, true);
    }
    batcher.endBatch();

    if (atlas) {
        raylib::Shader& alphaShader = AssetManager::getInstance().getShader("alphaCutout");
        ::BeginShaderMode(alphaShader);

        batcher.beginBatch(atlas->id);
        for (const auto& face : topFaceBatches) {
            batcher.addTopFaceUV(face.pos, 1.0f, 0.0f, 1.0f, face.u, face.v, face.uw, face.vh,
                                 face.color);
        }
        batcher.endBatch();

        // Draw decals without writing to the depth buffer to avoid Z-fighting
        // while allowing them to be drawn at the exact same height as the base tile
        // to prevent perspective shift gaps
        ::rlDisableDepthMask();
        batcher.beginBatch(atlas->id);
        for (const auto& decal : decalBatches) {
            batcher.addTopFaceUV(decal.pos, 1.0f, 0.0f, 1.0f, decal.u, decal.v, decal.uw, decal.vh,
                                 decal.color);
        }
        batcher.endBatch();
        ::rlEnableDepthMask();

        ::EndShaderMode();
    }
}

void RenderSystem::_renderLandmarks(World& w) {
    // Placeholder for monoliths, fissures, etc.
}

} // namespace zappy
