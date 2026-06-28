/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** VoxelBatcher.hpp
*/

#ifndef ZAPPY_VOXELBATCHER_HPP
#define ZAPPY_VOXELBATCHER_HPP

#include "raylib-cpp.hpp"

namespace zappy {

class VoxelBatcher {
  public:
    VoxelBatcher() = default;
    ~VoxelBatcher() = default;

    void beginBatch(unsigned int textureId);
    void beginSolidBatch();
    void addTopFace(const raylib::Vector3& pos, float w, float h, float l);
    void addTopFaceUV(const raylib::Vector3& pos, float w, float h, float l, float u, float v,
                      float uw, float vh, const raylib::Color& color = {255, 255, 255, 255});
    void addSideFaces(const raylib::Vector3& pos, float w, float h, float l,
                      const raylib::Color& color, bool drawFront, bool drawBack, bool drawLeft,
                      bool drawRight);
    void endBatch();

  private:
    bool _batchActive = false;
};

} // namespace zappy

#endif // ZAPPY_VOXELBATCHER_HPP
