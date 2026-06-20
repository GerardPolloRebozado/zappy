#include "Systems/RenderSystemBatches.hpp"
#include "Color.hpp"
#include <raymath.h>

namespace zappy {
namespace render {

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

} // namespace render
} // namespace zappy
