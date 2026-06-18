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

#include "Systems/RenderSystem.hpp"
#include "Color.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
#include "Components/FollowingEntity.hpp"
#include "Core.hpp"
#include "ECS/World.hpp"
#include "Graphics/TileTextures.hpp"
#include "Graphics/VoxelBatcher.hpp"
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

namespace {
struct InstanceKey {
    std::string modelName;
    unsigned int tint;
    bool operator==(const InstanceKey& o) const {
        return modelName == o.modelName && tint == o.tint;
    }
};

struct InstanceKeyHash {
    std::size_t operator()(const InstanceKey& k) const {
        return std::hash<std::string>{}(k.modelName) ^ std::hash<unsigned int>{}(k.tint);
    }
};

std::unordered_map<InstanceKey, std::vector<Matrix>, InstanceKeyHash> g_instanceBatches;

// Computes and queues an instance transform matrix for batched rendering.
// Scales, rotates, and translates the instance, multiplying the result
// by the base model transform to produce a single transformation matrix.
// This matrix is then queued into the rendering batch for later instancing.
void addInstance(const std::string& modelName, raylib::Vector3 pos, raylib::Vector3 axis,
                 float angle, raylib::Vector3 scale, raylib::Color tint,
                 const Matrix& modelTransform) {
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRot = MatrixRotate(axis, angle * DEG2RAD);
    Matrix matTrans = MatrixTranslate(pos.x, pos.y, pos.z);

    // Combine transformations: Scale * Rotation * Translation
    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRot), matTrans);

    // Apply any existing base model transform
    Matrix finalTransform = MatrixMultiply(modelTransform, matTransform);
    g_instanceBatches[{modelName, (unsigned int)ColorToInt(tint)}].push_back(finalTransform);
}
} // namespace

namespace zappy {

RenderSystem::RenderSystem() {
    _camera.position = (raylib::Vector3){20.0f, 20.0f, 20.0f};
    _camera.target = (raylib::Vector3){0.0f, 0.0f, 0.0f};
    _camera.up = (raylib::Vector3){0.0f, 1.0f, 0.0f};
    _camera.fovy = 45.0f;
    _camera.projection = CAMERA_PERSPECTIVE;
}

void RenderSystem::centerCamera(int width, int height) {
    _camera.target = (raylib::Vector3){(float)width / 2.0f, 0.0f, (float)height / 2.0f};
    _camera.position =
        (raylib::Vector3){(float)width / 2.0f + 10.0f, 15.0f, (float)height / 2.0f + 10.0f};
}

void RenderSystem::update(World& w, float dt) {
    (void)w;
    _handleInput(dt);
    _updateHoverState();
}

void RenderSystem::render(World& w) {
    _lazyLoadAssets();
    g_instanceBatches.clear();

    _camera.BeginMode();
    _renderTerrain(w);
    _renderLandmarks(w);
    _renderResources(w);
    _renderInhabitants(w);
    _renderEggs(w);
    _renderPOV(w);
    _renderParticles(w);

    // Hardware Instancing Rendering Phase
    // Iterate through batches of grouped models and pass their accumulated
    // transformation matrices to the GPU via DrawMeshInstanced, significantly
    for (auto& [key, transforms] : g_instanceBatches) {
        if (transforms.empty()) {
            continue;
        }
        raylib::Model& model = AssetManager::getInstance().getModel(key.modelName);
        Color tint = GetColor(key.tint);

        for (int i = 0; i < model.meshCount; i++) {
            Color oldColor =
                model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;
            Color combinedColor = {(unsigned char)((oldColor.r * tint.r) / 255),
                                   (unsigned char)((oldColor.g * tint.g) / 255),
                                   (unsigned char)((oldColor.b * tint.b) / 255),
                                   (unsigned char)((oldColor.a * tint.a) / 255)};

            // Skip tinting the head mesh (index 3) of the mannequin so it stays white
            if (key.modelName == "robot" && i == 3) {
                combinedColor = oldColor;
            }

            model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = combinedColor;

            DrawMeshInstanced(model.meshes[i], model.materials[model.meshMaterial[i]],
                              transforms.data(), transforms.size());

            model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = oldColor;
        }
    }

    _camera.EndMode();

    if (_showDebugHud) {
        _renderDebugHud(w);
    }
}

void RenderSystem::_lazyLoadAssets() {
    static bool loaded = false;
    if (!loaded) {
        AssetManager::getInstance().loadAll();
        raylib::Window::HideCursor();
        loaded = true;
    }
}

void RenderSystem::_handleInput(float dt) {
    float moveSpeed = 20.0f * dt;
    float rotateSpeed = 2.0f * dt;

    raylib::Vector3 forward = (raylib::Vector3)_camera.target - (raylib::Vector3)_camera.position;
    raylib::Vector3 right = forward.CrossProduct(_camera.up).Normalize();

    // Yaw Rotation (Q/E)
    if (raylib::Keyboard::IsKeyDown(KEY_Q)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        relPos = ::Vector3RotateByAxisAngle(relPos, {0, 1, 0}, rotateSpeed);
        _camera.position = (raylib::Vector3)_camera.target + relPos;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_E)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        relPos = ::Vector3RotateByAxisAngle(relPos, {0, 1, 0}, -rotateSpeed);
        _camera.position = (raylib::Vector3)_camera.target + relPos;
    }

    // Pitch Rotation (R/F)
    if (raylib::Keyboard::IsKeyDown(KEY_R)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        raylib::Vector3 nextRelPos = ::Vector3RotateByAxisAngle(relPos, right, -rotateSpeed);
        if (::Vector3Angle(nextRelPos, {0, 1, 0}) > 0.1f) {
            _camera.position = (raylib::Vector3)_camera.target + nextRelPos;
        }
    }
    if (raylib::Keyboard::IsKeyDown(KEY_F)) {
        raylib::Vector3 relPos =
            (raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target;
        raylib::Vector3 nextRelPos = ::Vector3RotateByAxisAngle(relPos, right, rotateSpeed);
        if (::Vector3Angle(nextRelPos, {0, 1, 0}) < 1.5f) {
            _camera.position = (raylib::Vector3)_camera.target + nextRelPos;
        }
    }

    // Panning (WASD)
    forward = (raylib::Vector3)_camera.target - (raylib::Vector3)_camera.position;
    forward.y = 0;
    forward = forward.Normalize();
    right = forward.CrossProduct(_camera.up);

    if (raylib::Keyboard::IsKeyDown(KEY_W)) {
        _camera.position = (raylib::Vector3)_camera.position + forward * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target + forward * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_S)) {
        _camera.position = (raylib::Vector3)_camera.position - forward * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target - forward * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_A)) {
        _camera.position = (raylib::Vector3)_camera.position - right * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target - right * moveSpeed;
    }
    if (raylib::Keyboard::IsKeyDown(KEY_D)) {
        _camera.position = (raylib::Vector3)_camera.position + right * moveSpeed;
        _camera.target = (raylib::Vector3)_camera.target + right * moveSpeed;
    }

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT)) {
        _selectedX = _hoveredX;
        _selectedZ = _hoveredZ;
    }

    _showDebugHud =
        raylib::Keyboard::IsKeyDown(KEY_LEFT_SHIFT) || raylib::Keyboard::IsKeyDown(KEY_RIGHT_SHIFT);

    // Zoom (Mouse Wheel)
    float wheel = raylib::Mouse::GetWheelMove();
    if (wheel != 0) {
        raylib::Ray ray = _camera.GetMouseRay(raylib::Mouse::GetPosition());
        if (ray.direction.y != 0) {
            float t = (1.5f - ray.position.y) / ray.direction.y;
            if (t > 0) {
                raylib::Vector3 mousePoint =
                    (raylib::Vector3)ray.position + (raylib::Vector3)ray.direction * t;
                raylib::Vector3 zoomVec =
                    (mousePoint - (raylib::Vector3)_camera.position) * (wheel * 0.1f);
                float nextDist = ((raylib::Vector3)_camera.position + zoomVec).Distance(mousePoint);
                if (nextDist > 2.0f && nextDist < 100.0f) {
                    _camera.position = (raylib::Vector3)_camera.position + zoomVec;
                    _camera.target = (raylib::Vector3)_camera.target + zoomVec;
                }
            }
        }
    }
}

void RenderSystem::_updateHoverState() {
    raylib::Ray mouseRay = _camera.GetMouseRay(raylib::Mouse::GetPosition());
    _hoveredX = InvalidTileCoord;
    _hoveredZ = InvalidTileCoord;
    if (mouseRay.direction.y != 0) {
        float t = (2.0f - mouseRay.position.y) / mouseRay.direction.y;
        if (t > 0) {
            raylib::Vector3 p =
                (raylib::Vector3)mouseRay.position + (raylib::Vector3)mouseRay.direction * t;
            _hoveredX = (int)std::round(p.x);
            _hoveredZ = (int)std::round(p.z);
        }
    }
}

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
            topFaceBatches.push_back({planePos, u, v, uw, vh});

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
                    decalBatches.push_back({decalPos, du, dv, duw, dvh});
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

            if (type->current_type == TerrainType::FOREST) {
                std::string treeNames[] = {"tree_1", "tree_2", "tree_group_1", "tree_group_2"};
                int treeIdx = (pos->x * 12345 + pos->y * 67890) % 4;
                if (treeIdx < 0) {
                    treeIdx += 4;
                }
                std::string treeKey = treeNames[treeIdx];

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

                        addInstance(treeKey, drawPos, {0, 1, 0}, 0.0f, {scale, scale, scale}, WHITE,
                                    treeModel.transform);
                    }
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
            batcher.addTopFaceUV(face.pos, 1.0f, 0.0f, 1.0f, face.u, face.v, face.uw, face.vh);
        }
        batcher.endBatch();

        // Draw decals without writing to the depth buffer to avoid Z-fighting
        // while allowing them to be drawn at the exact same height as the base tile
        // to prevent perspective shift gaps
        ::rlDisableDepthMask();
        batcher.beginBatch(atlas->id);
        for (const auto& decal : decalBatches) {
            batcher.addTopFaceUV(decal.pos, 1.0f, 0.0f, 1.0f, decal.u, decal.v, decal.uw, decal.vh);
        }
        batcher.endBatch();
        ::rlEnableDepthMask();

        ::EndShaderMode();
    }
}

void RenderSystem::_renderEggs(World& w) {
    auto eggStorage = w.get_storage<Egg>();
    if (!eggStorage) {
        return;
    }

    raylib::Model& eggModel = AssetManager::getInstance().getModel("egg");
    constexpr float scale = 0.1f;

    for (const auto& entity : *eggStorage | std::views::keys) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            const raylib::Vector3 vpos(static_cast<float>(pos->x), 2.3f,
                                       static_cast<float>(pos->y));
            addInstance("egg", vpos, {0, 1, 0}, 0.0f, {scale, scale, scale}, WHITE,
                        eggModel.transform);
        }
    }
}

void RenderSystem::_renderParticles(World& w) {
    auto emitterStorage = w.get_storage<ComponentParticleEmitter>();
    if (!emitterStorage) {
        return;
    }

    raylib::Vector3 forward =
        ((raylib::Vector3)_camera.position - (raylib::Vector3)_camera.target).Normalize();
    raylib::Vector3 right = ((raylib::Vector3)_camera.up).CrossProduct(forward).Normalize();
    raylib::Vector3 up = forward.CrossProduct(right).Normalize();

    ::rlDisableDepthMask();

    for (auto const& [entity, emitterPtr] : *emitterStorage) {
        if (emitterPtr->particles.empty()) {
            continue;
        }

        bool hasTexture = !emitterPtr->spriteName.empty();
        unsigned int texID = 0;
        if (hasTexture) {
            auto& tex = AssetManager::getInstance().getTexture(emitterPtr->spriteName);
            texID = tex.id;
        }

        if (hasTexture) {
            ::rlSetTexture(texID);
        }

        ::rlBegin(RL_QUADS);

        for (const auto& p : emitterPtr->particles) {
            float lifePct = 1.0f - (p.lifeRemaining / p.lifetime);
            if (lifePct < 0.0f) {
                lifePct = 0.0f;
            }
            if (lifePct > 1.0f) {
                lifePct = 1.0f;
            }

            unsigned char r =
                (unsigned char)(p.startColor.r + (p.endColor.r - p.startColor.r) * lifePct);
            unsigned char g =
                (unsigned char)(p.startColor.g + (p.endColor.g - p.startColor.g) * lifePct);
            unsigned char b =
                (unsigned char)(p.startColor.b + (p.endColor.b - p.startColor.b) * lifePct);
            unsigned char a =
                (unsigned char)(p.startColor.a + (p.endColor.a - p.startColor.a) * lifePct);

            float s = (p.startSize + (p.endSize - p.startSize) * lifePct) / 2.0f;
            float x = p.position.x;
            float y = p.position.y;
            float z = p.position.z;

            ::rlColor4ub(r, g, b, a);

            if (hasTexture) {
                raylib::Vector3 p1 = p.position - right * s - up * s;
                raylib::Vector3 p2 = p.position + right * s - up * s;
                raylib::Vector3 p3 = p.position + right * s + up * s;
                raylib::Vector3 p4 = p.position - right * s + up * s;

                ::rlTexCoord2f(0.0f, 1.0f);
                ::rlVertex3f(p1.x, p1.y, p1.z);
                ::rlTexCoord2f(1.0f, 1.0f);
                ::rlVertex3f(p2.x, p2.y, p2.z);
                ::rlTexCoord2f(1.0f, 0.0f);
                ::rlVertex3f(p3.x, p3.y, p3.z);
                ::rlTexCoord2f(0.0f, 0.0f);
                ::rlVertex3f(p4.x, p4.y, p4.z);
            } else {
                // Front
                ::rlVertex3f(x - s, y - s, z + s);
                ::rlVertex3f(x + s, y - s, z + s);
                ::rlVertex3f(x + s, y + s, z + s);
                ::rlVertex3f(x - s, y + s, z + s);

                // Back
                ::rlVertex3f(x - s, y - s, z - s);
                ::rlVertex3f(x - s, y + s, z - s);
                ::rlVertex3f(x + s, y + s, z - s);
                ::rlVertex3f(x + s, y - s, z - s);

                // Top
                ::rlVertex3f(x - s, y + s, z - s);
                ::rlVertex3f(x - s, y + s, z + s);
                ::rlVertex3f(x + s, y + s, z + s);
                ::rlVertex3f(x + s, y + s, z - s);

                // Bottom
                ::rlVertex3f(x - s, y - s, z - s);
                ::rlVertex3f(x + s, y - s, z - s);
                ::rlVertex3f(x + s, y - s, z + s);
                ::rlVertex3f(x - s, y - s, z + s);

                // Right
                ::rlVertex3f(x + s, y - s, z - s);
                ::rlVertex3f(x + s, y + s, z - s);
                ::rlVertex3f(x + s, y + s, z + s);
                ::rlVertex3f(x + s, y - s, z + s);

                // Left
                ::rlVertex3f(x - s, y - s, z - s);
                ::rlVertex3f(x - s, y - s, z + s);
                ::rlVertex3f(x - s, y + s, z + s);
                ::rlVertex3f(x - s, y + s, z - s);
            }
        }
        ::rlEnd();

        if (hasTexture) {
            ::rlSetTexture(0);
        }
    }

    ::rlEnableDepthMask();
}

void RenderSystem::_renderInhabitants(World& w) {
    auto orientationStorage = w.get_storage<Orientation>();
    if (!orientationStorage) {
        return;
    }

    auto& am = AssetManager::getInstance();
    raylib::Model& robot = const_cast<raylib::Model&>(am.getModel("robot"));
    constexpr float scale = 0.8f;

    // Get server frequency
    float freq = 100.0f;
    auto timeStorage = w.get_storage<TimeUnit>();
    if (timeStorage && timeStorage->begin() != timeStorage->end()) {
        freq = static_cast<float>(timeStorage->begin()->second->frequency);
    }
    if (freq <= 0.0f) {
        freq = 1.0f;
    }

    // Get bounding box to center the model
    BoundingBox box = robot.GetBoundingBox();
    raylib::Vector3 center = {(box.min.x + box.max.x) / 2.0f,
                              box.min.y, // Ground it on its bottom
                              (box.min.z + box.max.z) / 2.0f};

    std::set<std::string> teamNames;
    auto teamStorage = w.get_storage<TeamName>();
    if (teamStorage) {
        for (auto const& [e, t] : *teamStorage) {
            teamNames.insert(t->_team_name);
        }
    }

    for (auto const& [entity, orientationPtr] : *orientationStorage) {
        auto pos = w.get_component<Position>(entity);
        auto move = w.get_component<MovementInterpolation>(entity);
        auto anim = w.get_component<Animation>(entity);
        if (pos && move) {
            if (anim) {
                try {
                    auto& modelAnim = am.getAnimation(anim->currentAnim);
                    UpdateModelAnimation(robot, modelAnim, static_cast<int>(anim->currentFrame));
                } catch (...) {
                }
            }
            float rotation = 0.0f;
            switch (orientationPtr->current_direction) {
                case Orientation::N:
                    rotation = 270.0f - 90.0f;
                    break;
                case Orientation::E:
                    rotation = 180.0f - 90.0f;
                    break;
                case Orientation::S:
                    rotation = 90.0f - 90.0f;
                    break;
                case Orientation::W:
                    rotation = 0.0f - 90.0f;
                    break;
            }

            raylib::Vector3 centerOffset = {center.x * scale, center.y * scale, center.z * scale};
            centerOffset = ::Vector3RotateByAxisAngle(centerOffset, {0, 1, 0}, rotation * DEG2RAD);

            raylib::Vector3 vpos(move->visualX - centerOffset.x, 2.01f - centerOffset.y,
                                 move->visualY - centerOffset.z);

            auto team = w.get_component<TeamName>(entity);
            if (team) {
                raylib::Color tColor = team->_color;

                // Draw mannequin manually to keep head white
                Matrix matScale = MatrixScale(scale, scale, scale);
                Matrix matRotation = MatrixRotate({0, 1, 0}, rotation * DEG2RAD);
                Matrix matTranslation = MatrixTranslate(vpos.x, vpos.y, vpos.z);
                Matrix matTransform =
                    MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

                for (int i = 0; i < robot.meshCount; i++) {
                    Material mat = robot.materials[robot.meshMaterial[i]];
                    Color drawColor = tColor;
                    if (i == 3) {
                        drawColor = WHITE; // Head is white
                    } else {
                        drawColor = tColor;
                    }
                    mat.maps[MATERIAL_MAP_DIFFUSE].color = drawColor;
                    ::DrawMesh(robot.meshes[i], mat, matTransform);
                }

                float wH = 0.12f;
                float wT = 0.12f;
                float yB = 2.02f;
                float dist = 0.38f;
                raylib::Vector3(move->visualX, yB + wH / 2, move->visualY + dist)
                    .DrawCube(dist * 2 + wT, wH, wT, tColor);
                raylib::Vector3(move->visualX, yB + wH / 2, move->visualY - dist)
                    .DrawCube(dist * 2 + wT, wH, wT, tColor);
                raylib::Vector3(move->visualX + dist, yB + wH / 2, move->visualY)
                    .DrawCube(wT, wH, dist * 2 + wT, tColor);
                raylib::Vector3(move->visualX - dist, yB + wH / 2, move->visualY)
                    .DrawCube(wT, wH, dist * 2 + wT, tColor);
            }
        }
    }
}

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

                    addInstance(selectedFood, cPos, {0, 1, 0}, rotAngle, {scale, scale, scale},
                                WHITE, foodModel.transform);
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

                addInstance(modelName, drawPos, {0, 1, 0}, rotAngle,
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

void RenderSystem::_renderLandmarks(World& w) {
    // Placeholder for monoliths, fissures, etc.
}

void RenderSystem::_renderHoverEffect(int x, int z) {
    raylib::Color hCol = raylib::Color::White();
    float wH = 0.15f;
    float wT = 0.05f;
    float yB = 2.0f;

    raylib::Vector3((float)x, yB + wH / 2, (float)z + 0.5f).DrawCube(1.0f + wT, wH, wT, hCol);
    raylib::Vector3((float)x, yB + wH / 2, (float)z - 0.5f).DrawCube(1.0f + wT, wH, wT, hCol);
    raylib::Vector3((float)x + 0.5f, yB + wH / 2, (float)z).DrawCube(wT, wH, 1.0f + wT, hCol);
    raylib::Vector3((float)x - 0.5f, yB + wH / 2, (float)z).DrawCube(wT, wH, 1.0f + wT, hCol);
}

void RenderSystem::_renderDebugHud(World& w) {
    int padding = 15;
    int y = padding;
    int x = padding;
    int spacing = 25;

    auto drawText = [&](const std::string& text) {
        ::DrawText(text.c_str(), x, y, 20, WHITE);
        y += spacing;
    };

    ::DrawRectangle(5, 5, 450, 300, raylib::Color(0, 0, 0, 150));

    drawText("--- DEBUG HUD (HOLD SHIFT) ---");
    drawText("FPS: " + std::to_string(::GetFPS()));

    char camPos[64];
    snprintf(camPos, sizeof(camPos), "Camera Pos: %.2f, %.2f, %.2f", _camera.position.x,
             _camera.position.y, _camera.position.z);
    drawText(camPos);

    char camTgt[64];
    snprintf(camTgt, sizeof(camTgt), "Camera Tgt: %.2f, %.2f, %.2f", _camera.target.x,
             _camera.target.y, _camera.target.z);
    drawText(camTgt);

    drawText("Hovered Tile: " + std::to_string(_hoveredX) + ", " + std::to_string(_hoveredZ));
    drawText("Selected Tile: " + std::to_string(_selectedX) + ", " + std::to_string(_selectedZ));

    auto terrainStorage = w.get_storage<TerrainType>();
    drawText("Tiles Loaded: " + std::to_string(terrainStorage ? terrainStorage->size() : 0));

    auto inhabitantStorage = w.get_storage<Orientation>();
    drawText("Inhabitants: " + std::to_string(inhabitantStorage ? inhabitantStorage->size() : 0));

    auto eggStorage = w.get_storage<Egg>();
    drawText("Eggs on Map: " + std::to_string(eggStorage ? eggStorage->size() : 0));
}

void RenderSystem::_renderPOV(World& w) {
    auto followingEntity = w.get_storage<FollowingEntity>();
    if (!followingEntity || followingEntity->size() == 0) {
        return;
    }

    if (raylib::Keyboard::IsKeyDown(KEY_W) || raylib::Keyboard::IsKeyDown(KEY_A) ||
        raylib::Keyboard::IsKeyDown(KEY_S) || raylib::Keyboard::IsKeyDown(KEY_D)) {
        followingEntity->clear();
        return;
    }
    auto entity = followingEntity->begin()->first;
    auto pos = w.get_component<Position>(entity);
    auto orientation = w.get_component<Orientation>(entity);
    if (!pos || !orientation) {
        return;
    }

    Vector3 headPos = {static_cast<float>(pos->x), 3.0f, static_cast<float>(pos->y)};
    Vector3 lookDir = {0.0f, 0.0f, 0.0f};

    switch (orientation->current_direction) {
        case Orientation::N:
            lookDir = {0.0f, 0.0f, -1.0f};
            break;
        case Orientation::E:
            lookDir = {1.0f, 0.0f, 0.0f};
            break;
        case Orientation::S:
            lookDir = {0.0f, 0.0f, 1.0f};
            break;
        case Orientation::W:
            lookDir = {-1.0f, 0.0f, 0.0f};
            break;
    }

    _camera.position = headPos;
    _camera.target = Vector3Add(headPos, lookDir);
}

} // namespace zappy
