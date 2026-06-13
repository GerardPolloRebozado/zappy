/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** TileTextures.hpp
*/

#ifndef ZAPPY_TILETEXTURES_HPP
#define ZAPPY_TILETEXTURES_HPP

#include "Components/ComponentTile.hpp"
#include "raylib-cpp.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace zappy {

// Caches textures because generating them pixel by pixel every frame is too slow
// The key includes all 8 neighbor types. This way, if a missing tile later arrives
// from the network, the cache invalidates and the border automatically draws
struct TileCacheKey {
    int variation;
    int types[9]; // self + 8 neighbors
    bool operator==(const TileCacheKey& other) const {
        if (variation != other.variation) {
            return false;
        }
        for (int i = 0; i < 9; ++i) {
            if (types[i] != other.types[i]) {
                return false;
            }
        }
        return true;
    }
};

// Combines tile coordinates and neighbor types into a unique hash key
struct TileCacheKeyHash {
    std::size_t operator()(const TileCacheKey& k) const {
        std::size_t h = std::hash<int>{}(k.variation);
        for (int i = 0; i < 9; ++i) {
            h ^= std::hash<int>{}(k.types[i]) << (i % 8);
        }
        return h;
    }
};

class Tiletextures {
    std::unordered_map<TerrainType::Type, std::vector<raylib::Color>> _colors = {
        {TerrainType::GRASS,
         {raylib::Color(110, 185, 70), raylib::Color(125, 200, 80), raylib::Color(95, 165, 55),
          raylib::Color(80, 145, 45)}},
        {TerrainType::MOUNTAIN,
         {raylib::Color(80, 85, 90), raylib::Color(105, 110, 115), raylib::Color(65, 68, 72),
          raylib::Color(50, 52, 55)}},
        {TerrainType::WATER,
         {raylib::Color(40, 90, 200), raylib::Color(65, 120, 230), raylib::Color(30, 70, 170),
          raylib::Color(110, 175, 245)}},
        {TerrainType::SAND,
         {raylib::Color(220, 180, 90), raylib::Color(240, 200, 115), raylib::Color(195, 155, 70),
          raylib::Color(170, 130, 55)}},
        {TerrainType::FOREST,
         {raylib::Color(35, 115, 50), raylib::Color(55, 140, 70), raylib::Color(25, 90, 35),
          raylib::Color(15, 65, 20)}},
        {TerrainType::OBSIDIAN_BARRENS,
         {raylib::Color(25, 25, 30), raylib::Color(45, 40, 50), raylib::Color(15, 15, 18),
          raylib::Color(75, 30, 35)}},
        {TerrainType::LUMINOUS_ORCHARDS,
         {raylib::Color(45, 205, 90), raylib::Color(90, 245, 135), raylib::Color(30, 165, 75),
          raylib::Color(0, 235, 215)}},
        {TerrainType::CRYSTAL_CANYONS,
         {raylib::Color(135, 60, 185), raylib::Color(170, 95, 220), raylib::Color(100, 40, 145),
          raylib::Color(225, 135, 245)}},
        {TerrainType::MAGNETIC_TUNDRA,
         {raylib::Color(145, 205, 235), raylib::Color(190, 230, 250), raylib::Color(115, 175, 205),
          raylib::Color(240, 250, 255)}}};
    std::unordered_map<TileCacheKey, std::shared_ptr<raylib::Texture2D>, TileCacheKeyHash>
        _tileTextures;

  public:
    std::shared_ptr<raylib::Texture2D>
    GetTileTexture(int x, int y, TerrainType::Type type,
                   const std::unordered_map<uint64_t, TerrainType::Type>& mapGrid) {

        auto getMapType = [&](int nx, int ny) -> int {
            uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(nx)) << 32) |
                           static_cast<uint32_t>(ny);
            if (mapGrid.count(key)) {
                return static_cast<int>(mapGrid.at(key));
            }
            return -1;
        };

        TileCacheKey key;
        int variation = (x * 73856093 ^ y * 19349663) % 4; // 4 visual variations
        if (variation < 0) {
            variation += 4;
        }
        key.variation = variation;
        key.types[0] = static_cast<int>(type);
        key.types[1] = getMapType(x - 1, y);
        key.types[2] = getMapType(x + 1, y);
        key.types[3] = getMapType(x, y - 1);
        key.types[4] = getMapType(x, y + 1);
        key.types[5] = getMapType(x - 1, y - 1);
        key.types[6] = getMapType(x + 1, y - 1);
        key.types[7] = getMapType(x - 1, y + 1);
        key.types[8] = getMapType(x + 1, y + 1);

        if (_tileTextures.find(key) != _tileTextures.end()) {
            return _tileTextures[key];
        }

        constexpr int size = 32;
        raylib::Image image = GenImageColor(size, size, BLANK);

        // Generates consistent noise so neighbor tiles always match perfectly at their borders
        auto fast_hash = [](uint32_t a, uint32_t b) {
            uint32_t h = (a * 374761393) ^ (b * 668265263);
            h = (h ^ (h >> 16)) * 2246822507;
            h = (h ^ (h >> 13)) * 3266489909;
            return h ^ (h >> 16);
        };

        auto getPriority = [](TerrainType::Type t) {
            switch (t) {
                case TerrainType::WATER:
                    return 90;
                case TerrainType::MOUNTAIN:
                    return 80;
                case TerrainType::FOREST:
                    return 70;
                case TerrainType::GRASS:
                    return 60;
                case TerrainType::SAND:
                    return 50;
                case TerrainType::OBSIDIAN_BARRENS:
                    return 40;
                case TerrainType::LUMINOUS_ORCHARDS:
                    return 30;
                case TerrainType::CRYSTAL_CANYONS:
                    return 20;
                case TerrainType::MAGNETIC_TUNDRA:
                    return 10;
                default:
                    return 0;
            }
        };

        for (int py = 0; py < size; py++) {
            for (int px = 0; px < size; px++) {
                // Incorporate variation to offset the noise pattern so tiles don't look completely
                // identical
                int local_nx = px + variation * size;
                int local_ny = py + variation * size;

                TerrainType::Type pixelType = type;
                int currentPriority = getPriority(type);

                auto checkBleed = [&](int nx, int ny, int dist) {
                    uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(nx)) << 32) |
                                   static_cast<uint32_t>(ny);
                    if (!mapGrid.count(key)) {
                        return;
                    }
                    TerrainType::Type nType = mapGrid.at(key);
                    int nPriority = getPriority(nType);

                    if (nPriority > currentPriority) {
                        uint32_t noise = fast_hash(local_nx, local_ny);
                        if (dist < 3 + (noise % 6)) {
                            pixelType = nType;
                            currentPriority = nPriority;
                        }
                    }
                };

                // Check orthogonal neighbors (up, down, left, right)
                // The distance is simply how close the pixel is to the flat edge
                checkBleed(x - 1, y, px);              // left (-x)
                checkBleed(x + 1, y, (size - 1) - px); // right (+x)
                checkBleed(x, y - 1, py);              // top (-z)
                checkBleed(x, y + 1, (size - 1) - py); // bottom (+z)

                // Check diagonal neighbors to smoothly blend the 4 corners of the tile
                // We use Chebyshev distance (std::max) because it correctly calculates
                // distance to a corner on a square grid, giving organic rounded edges
                // https://www.chessprogramming.org/index.php?title=Distance&diff=cur&oldid=5917
                checkBleed(x - 1, y - 1, std::max(px, py));
                checkBleed(x + 1, y - 1, std::max((size - 1) - px, py));
                checkBleed(x - 1, y + 1, std::max(px, (size - 1) - py));
                checkBleed(x + 1, y + 1, std::max((size - 1) - px, (size - 1) - py));

                const auto& palette = _colors[pixelType];
                const uint32_t randVal = fast_hash(local_nx, local_ny) % 100;
                raylib::Color pixelColor;

                if (randVal < 55) {
                    pixelColor = palette[0];
                } else if (randVal < 80) {
                    pixelColor = palette[1];
                } else if (randVal < 93) {
                    pixelColor = palette[2];
                } else {
                    pixelColor = palette[3];
                }

                image.DrawPixel(px, py, pixelColor);
            }
        }

        raylib::Texture2D texture(image);
        texture.SetFilter(TEXTURE_FILTER_POINT);

        auto shared_tex = std::make_shared<raylib::Texture2D>(std::move(texture));
        _tileTextures[key] = shared_tex;
        return shared_tex;
    }

    Tiletextures() = default;

    ~Tiletextures() = default;
};

} // namespace zappy

#endif // ZAPPY_TILETEXTURES_HPP
