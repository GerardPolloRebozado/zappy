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
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
#include "ECS/World.hpp"
#include "Graphics/TileTextures.hpp"
#include "Graphics/VoxelBatcher.hpp"

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

// Adds a 3D model to the giant batch for drawing later
void addInstance(const std::string& modelName, raylib::Vector3 pos, raylib::Vector3 axis,
                 float angle, raylib::Vector3 scale, raylib::Color tint,
                 const Matrix& modelTransform) {
    // Calculate the math to resize, spin, and place the object
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRot = MatrixRotate(axis, angle * DEG2RAD);
    Matrix matTrans = MatrixTranslate(pos.x, pos.y, pos.z);

    // Smash them all together into a single Matrix (order matters!!!!)
    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRot), matTrans);

    // Apply any base offsets the model already had, then save it to the batch list
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

    // Render all batched 3D models at once (Instancing)
    // We group identical models (like all "tree" objects) and pass their
    // positions/rotations as a giant list of matrices. This drastically
    // cuts down GPU draw calls and boosts performance.
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
                raylib::Model& treeModel = AssetManager::getInstance().getModel("tree2");
                auto& am = AssetManager::getInstance();
                std::shared_ptr<BoundingBox> box = am.getBoundingBox("tree2", treeModel);

                float sizeX = box->max.x - box->min.x;
                float sizeZ = box->max.z - box->min.z;

                if (sizeX > 0 && sizeZ > 0) {
                    bool playerOnTile = inhabitantPositions.count(hashPos(pos->x, pos->y)) > 0;

                    if (!playerOnTile) {
                        float scale = 0.4f / std::max(sizeX, sizeZ);

                        raylib::Vector3 centerOffset((box->max.x + box->min.x) / 2.0f * scale,
                                                     box->min.y * scale,
                                                     (box->max.z + box->min.z) / 2.0f * scale);

                        // Sink the tree by 0.1f to hide its built-in grass base
                        raylib::Vector3 drawPos(vpos.x - centerOffset.x,
                                                2.0f - centerOffset.y - 0.1f,
                                                vpos.z - centerOffset.z);

                        // Deterministic random offset within the tile
                        float rX = ((std::abs(pos->x * 137 + pos->y * 31)) % 61) / 100.0f - 0.3f;
                        float rZ = ((std::abs(pos->x * 19 + pos->y * 101)) % 61) / 100.0f - 0.3f;
                        drawPos.x += rX;
                        drawPos.z += rZ;

                        raylib::Model& treeModel = AssetManager::getInstance().getModel("tree2");
                        addInstance("tree2", drawPos, {0, 1, 0}, 0.0f, {scale, scale, scale}, WHITE,
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
    constexpr float scale = 0.02f;

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

void RenderSystem::_renderInhabitants(World& w) {
    auto orientationStorage = w.get_storage<Orientation>();
    if (!orientationStorage) {
        return;
    }

    auto& am = AssetManager::getInstance();
    const raylib::Model& robot = am.getModel("robot");
    constexpr float scale = 0.1f;

    // Get bounding box to center the model
    BoundingBox box = robot.GetBoundingBox();
    raylib::Vector3 center = {(box.min.x + box.max.x) / 2.0f,
                              box.min.y, // Ground it on its bottom
                              (box.min.z + box.max.z) / 2.0f};

    std::set<std::string> teamNames;
    auto teamStorage = w.get_storage<TeamName>();
    if (teamStorage) {
        for (auto const& [e, t] : *teamStorage) {
            teamNames.insert(t->team_name);
        }
    }

    for (auto const& [entity, orientationPtr] : *orientationStorage) {
        auto pos = w.get_component<Position>(entity);
        if (pos) {
            float rotation = 0.0f;
            switch (orientationPtr->current_direction) {
                case Orientation::N:
                    rotation = 270.0f;
                    break;
                case Orientation::E:
                    rotation = 180.0f;
                    break;
                case Orientation::S:
                    rotation = 90.0f;
                    break;
                case Orientation::W:
                    rotation = 0.0f;
                    break;
            }

            // Calculate position: center the model on the tile correctly even when rotated
            raylib::Vector3 centerOffset = {center.x * scale, center.y * scale, center.z * scale};
            centerOffset = ::Vector3RotateByAxisAngle(centerOffset, {0, 1, 0}, rotation * DEG2RAD);

            raylib::Vector3 vpos((float)pos->x - centerOffset.x, 2.01f - centerOffset.y,
                                 (float)pos->y - centerOffset.z);

            addInstance("robot", vpos, {0, 1, 0}, rotation, {scale, scale, scale}, WHITE,
                        robot.transform);

            auto team = w.get_component<TeamName>(entity);
            if (team) {
                int colorIndex = 0;
                for (const auto& tn : teamNames) {
                    if (tn == team->team_name) {
                        break;
                    }
                    colorIndex++;
                }

                std::vector<raylib::Color> teamColors = {
                    raylib::Color(230, 60, 60, 255),  // Red
                    raylib::Color(60, 230, 60, 255),  // Green
                    raylib::Color(60, 100, 230, 255), // Blue
                    raylib::Color(230, 230, 60, 255), // Yellow
                    raylib::Color(230, 60, 230, 255), // Magenta
                    raylib::Color(60, 230, 230, 255)  // Cyan
                };
                raylib::Color tColor = teamColors[colorIndex % teamColors.size()];

                float wH = 0.12f; // Thicker height
                float wT = 0.12f; // Thicker width
                float yB = 2.02f;
                float dist = 0.38f;
                raylib::Vector3((float)pos->x, yB + wH / 2, (float)pos->y + dist)
                    .DrawCube(dist * 2 + wT, wH, wT, tColor);
                raylib::Vector3((float)pos->x, yB + wH / 2, (float)pos->y - dist)
                    .DrawCube(dist * 2 + wT, wH, wT, tColor);
                raylib::Vector3((float)pos->x + dist, yB + wH / 2, (float)pos->y)
                    .DrawCube(wT, wH, dist * 2 + wT, tColor);
                raylib::Vector3((float)pos->x - dist, yB + wH / 2, (float)pos->y)
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
    std::vector<std::string> grassAnimals = {"bunny", "cat", "cow", "dog", "piglet"};
    std::vector<std::string> mountainAnimals = {"bear", "mole"};
    std::vector<std::string> waterAnimals = {"axolotl", "crocodile", "frog", "penguin", "turtle"};
    std::vector<std::string> sandAnimals = {"crocodile", "elephant", "turtle"};
    std::vector<std::string> forestAnimals = {"bear",  "bunny", "fox",   "monkey",
                                              "mouse", "panda", "parrot"};
    std::vector<std::string> obsidianAnimals = {"mole", "mouse", "crocodile"};
    std::vector<std::string> luminousAnimals = {"bunny", "frog", "parrot", "unicorn"};
    std::vector<std::string> crystalAnimals = {"bear", "fox", "mole"};
    std::vector<std::string> magneticAnimals = {"bear", "fox", "penguin"};

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

            // Use biome-specific voxel animals for food, offset slightly from center
            if (inv->food > 0) {
                std::vector<std::string>* pool = nullptr;

                switch (type->current_type) {
                    case TerrainType::GRASS:
                        pool = &grassAnimals;
                        break;
                    case TerrainType::MOUNTAIN:
                        pool = &mountainAnimals;
                        break;
                    case TerrainType::WATER:
                        pool = &waterAnimals;
                        break;
                    case TerrainType::SAND:
                        pool = &sandAnimals;
                        break;
                    case TerrainType::FOREST:
                        pool = &forestAnimals;
                        break;
                    case TerrainType::OBSIDIAN_BARRENS:
                        pool = &obsidianAnimals;
                        break;
                    case TerrainType::LUMINOUS_ORCHARDS:
                        pool = &luminousAnimals;
                        break;
                    case TerrainType::CRYSTAL_CANYONS:
                        pool = &crystalAnimals;
                        break;
                    case TerrainType::MAGNETIC_TUNDRA:
                        pool = &magneticAnimals;
                        break;
                }

                std::string selectedAnimal = "voxel_chicken"; // default
                if (pool && !pool->empty()) {
                    int index = (std::abs(pos->x * 137 + pos->y * 31)) % pool->size();
                    selectedAnimal = "voxel_" + (*pool)[index];
                }

                raylib::Model& foodModel = am.getModel(selectedAnimal);
                std::shared_ptr<BoundingBox> cBox = am.getBoundingBox(selectedAnimal, foodModel);

                float sizeX = cBox->max.x - cBox->min.x;
                float sizeZ = cBox->max.z - cBox->min.z;
                float sizeY = cBox->max.y - cBox->min.y;
                float maxDim = std::max({sizeX, sizeY, sizeZ});
                // Make animals smaller: max dim is 0.2f
                float scale = (maxDim > 0) ? (0.20f / maxDim) : 0.10f;

                raylib::Vector3 cCenter = {(cBox->min.x + cBox->max.x) / 2.0f, cBox->min.y,
                                           (cBox->min.z + cBox->max.z) / 2.0f};

                // Deterministic random offset for food within the tile
                float rX = ((std::abs(pos->x * 71 + pos->y * 13)) % 81) / 100.0f - 0.4f;
                float rZ = ((std::abs(pos->x * 23 + pos->y * 89)) % 81) / 100.0f - 0.4f;

                raylib::Vector3 cPos((float)pos->x + rX - (cCenter.x * scale),
                                     yBase - (cCenter.y * scale),
                                     (float)pos->y + rZ - (cCenter.z * scale));

                // Deterministic random rotation
                float rotAngle = ((std::abs(pos->x * 47 + pos->y * 59)) % 360);

                addInstance(selectedAnimal, cPos, {0, 1, 0}, rotAngle, {scale, scale, scale}, WHITE,
                            foodModel.transform);
            }

            auto drawGem = [&](const std::string& modelName, float dx, float dz,
                               raylib::Color tint) {
                raylib::Model& rockModel = am.getModel(modelName);
                auto box = am.getBoundingBox(modelName, rockModel);
                float sizeX = box->max.x - box->min.x;
                float sizeZ = box->max.z - box->min.z;
                float sizeY = box->max.y - box->min.y;
                float maxDim = std::max({sizeX, sizeY, sizeZ});
                float rockScale = (maxDim > 0) ? (0.15f / maxDim) : 0.05f;

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

            if (inv->linemate > 0) {
                drawGem("rock1", 0.0f, 0.0f, WHITE);
            }
            if (inv->deraumere > 0) {
                drawGem("rock2", 0.2f, 0.0f, SKYBLUE);
            }
            if (inv->sibur > 0) {
                drawGem("rock1", 0.4f, 0.0f, DARKBLUE);
            }
            if (inv->mendiane > 0) {
                drawGem("rock2", 0.0f, 0.2f, PINK);
            }
            if (inv->phiras > 0) {
                drawGem("rock1", 0.2f, 0.2f, DARKPURPLE);
            }
            if (inv->thystame > 0) {
                drawGem("rock2", 0.4f, 0.2f, GOLD);
            }
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

} // namespace zappy
