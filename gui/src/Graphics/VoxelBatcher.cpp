/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** VoxelBatcher.cpp
*/

#include "Graphics/VoxelBatcher.hpp"
#include <rlgl.h>

namespace zappy {

void VoxelBatcher::beginBatch(unsigned int textureId) {
    if (_batchActive) {
        endBatch();
    }
    rlSetTexture(textureId);
    rlBegin(RL_QUADS);
    _batchActive = true;
}

void VoxelBatcher::beginSolidBatch() {
    if (_batchActive) {
        endBatch();
    }
    rlSetTexture(0);
    rlBegin(RL_QUADS);
    _batchActive = true;
}

void VoxelBatcher::addTopFace(const raylib::Vector3& pos, float w, float h, float l) {
    if (!_batchActive) {
        return;
    }
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    rlColor4ub(255, 255, 255, 255);
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex3f(x - w / 2, y + h / 2, z - l / 2);
    rlTexCoord2f(0.0f, 1.0f);
    rlVertex3f(x - w / 2, y + h / 2, z + l / 2);
    rlTexCoord2f(1.0f, 1.0f);
    rlVertex3f(x + w / 2, y + h / 2, z + l / 2);
    rlTexCoord2f(1.0f, 0.0f);
    rlVertex3f(x + w / 2, y + h / 2, z - l / 2);
}

void VoxelBatcher::addTopFaceUV(const raylib::Vector3& pos, float w, float h, float l, float u,
                                float v, float uw, float vh) {
    if (!_batchActive) {
        return;
    }
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    rlColor4ub(255, 255, 255, 255);
    rlTexCoord2f(u, v);
    rlVertex3f(x - w / 2, y + h / 2, z - l / 2);
    rlTexCoord2f(u, v + vh);
    rlVertex3f(x - w / 2, y + h / 2, z + l / 2);
    rlTexCoord2f(u + uw, v + vh);
    rlVertex3f(x + w / 2, y + h / 2, z + l / 2);
    rlTexCoord2f(u + uw, v);
    rlVertex3f(x + w / 2, y + h / 2, z - l / 2);
}

void VoxelBatcher::addSideFaces(const raylib::Vector3& pos, float w, float h, float l,
                                const raylib::Color& color, bool drawFront, bool drawBack,
                                bool drawLeft, bool drawRight) {
    if (!_batchActive) {
        return;
    }
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    rlColor4ub(color.r, color.g, color.b, color.a);

    // Front face
    if (drawFront) {
        rlVertex3f(x - w / 2, y + h / 2, z + l / 2);
        rlVertex3f(x - w / 2, y - h / 2, z + l / 2);
        rlVertex3f(x + w / 2, y - h / 2, z + l / 2);
        rlVertex3f(x + w / 2, y + h / 2, z + l / 2);
    }

    // Back face
    if (drawBack) {
        rlVertex3f(x - w / 2, y + h / 2, z - l / 2);
        rlVertex3f(x + w / 2, y + h / 2, z - l / 2);
        rlVertex3f(x + w / 2, y - h / 2, z - l / 2);
        rlVertex3f(x - w / 2, y - h / 2, z - l / 2);
    }

    // Right face
    if (drawRight) {
        rlVertex3f(x + w / 2, y + h / 2, z + l / 2);
        rlVertex3f(x + w / 2, y - h / 2, z + l / 2);
        rlVertex3f(x + w / 2, y - h / 2, z - l / 2);
        rlVertex3f(x + w / 2, y + h / 2, z - l / 2);
    }

    // Left face
    if (drawLeft) {
        rlVertex3f(x - w / 2, y + h / 2, z + l / 2);
        rlVertex3f(x - w / 2, y + h / 2, z - l / 2);
        rlVertex3f(x - w / 2, y - h / 2, z - l / 2);
        rlVertex3f(x - w / 2, y - h / 2, z + l / 2);
    }
}

void VoxelBatcher::endBatch() {
    if (_batchActive) {
        rlEnd();
        _batchActive = false;
    }
}

} // namespace zappy
