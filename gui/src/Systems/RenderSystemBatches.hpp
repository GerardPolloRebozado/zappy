#pragma once
#include "raylib-cpp.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace zappy {
namespace render {

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

extern std::unordered_map<InstanceKey, std::vector<Matrix>, InstanceKeyHash> g_instanceBatches;

void addInstance(const std::string& modelName, raylib::Vector3 pos, raylib::Vector3 axis,
                 float angle, raylib::Vector3 scale, raylib::Color tint,
                 const Matrix& modelTransform);

} // namespace render
} // namespace zappy
