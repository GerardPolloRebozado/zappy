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

enum class DecalDirection : int {
    SOLID = 0,
    NORTH = 1,
    SOUTH = 2,
    EAST = 3,
    WEST = 4,
    NORTH_WEST = 5,
    NORTH_EAST = 6,
    SOUTH_WEST = 7,
    SOUTH_EAST = 8
};

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
         {raylib::Color(70, 55, 35), raylib::Color(85, 70, 45), raylib::Color(55, 45, 25),
          raylib::Color(45, 35, 20)}},
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
    std::shared_ptr<raylib::Texture2D> _atlas;

    void generateAtlas() {
        if (_atlas) {
            return;
        }
        constexpr int size = 32;
        constexpr int biomes = 9;
        constexpr int variations = 4;
        constexpr int cols = 9; // 0: Solid, 1: N, 2: S, 3: E, 4: W, 5: NW, 6: NE, 7: SW, 8: SE

        raylib::Image atlasImage = GenImageColor(cols * size, biomes * variations * size, BLANK);

        auto fast_hash = [](uint32_t a, uint32_t b) {
            uint32_t h = (a * 374761393) ^ (b * 668265263);
            h = (h ^ (h >> 16)) * 2246822507;
            h = (h ^ (h >> 13)) * 3266489909;
            return h ^ (h >> 16);
        };

        for (int b = 0; b < biomes; ++b) {
            TerrainType::Type biomeType = static_cast<TerrainType::Type>(b);
            const auto& palette = _colors[biomeType];

            for (int var = 0; var < variations; ++var) {
                int row = b * variations + var;

                for (int col = 0; col < cols; ++col) {
                    for (int py = 0; py < size; py++) {
                        for (int px = 0; px < size; px++) {
                            int noise_px = px;
                            int noise_py = py;

                            switch (static_cast<DecalDirection>(col)) {
                                case DecalDirection::SOLID:
                                    break;
                                case DecalDirection::NORTH:
                                    noise_py += size;
                                    break; // North bleed (extends South edge)
                                case DecalDirection::SOUTH:
                                    noise_py -= size;
                                    break; // South bleed (extends North edge)
                                case DecalDirection::EAST:
                                    noise_px -= size;
                                    break; // East bleed (extends West edge)
                                case DecalDirection::WEST:
                                    noise_px += size;
                                    break; // West bleed (extends East edge)
                                case DecalDirection::NORTH_WEST:
                                    noise_px += size;
                                    noise_py += size;
                                    break; // NW bleed
                                case DecalDirection::NORTH_EAST:
                                    noise_px -= size;
                                    noise_py += size;
                                    break; // NE bleed
                                case DecalDirection::SOUTH_WEST:
                                    noise_px += size;
                                    noise_py -= size;
                                    break; // SW bleed
                                case DecalDirection::SOUTH_EAST:
                                    noise_px -= size;
                                    noise_py -= size;
                                    break; // SE bleed
                            }

                            int local_nx = noise_px + var * size;
                            int local_ny = noise_py + var * size;

                            bool drawPixel = false;
                            int dist = 999;
                            int parallel = 0;

                            if (static_cast<DecalDirection>(col) == DecalDirection::SOLID) {
                                drawPixel = true;
                            } else {
                                switch (static_cast<DecalDirection>(col)) {
                                    case DecalDirection::SOLID:
                                        break;
                                    case DecalDirection::NORTH:
                                        dist = py;
                                        parallel = px;
                                        break;
                                    case DecalDirection::SOUTH:
                                        dist = (size - 1) - py;
                                        parallel = px;
                                        break;
                                    case DecalDirection::EAST:
                                        dist = (size - 1) - px;
                                        parallel = py;
                                        break;
                                    case DecalDirection::WEST:
                                        dist = px;
                                        parallel = py;
                                        break;
                                    case DecalDirection::NORTH_WEST:
                                        dist = std::max(px, py);
                                        parallel = px - py;
                                        break;
                                    case DecalDirection::NORTH_EAST:
                                        dist = std::max((size - 1) - px, py);
                                        parallel = px + py;
                                        break;
                                    case DecalDirection::SOUTH_WEST:
                                        dist = std::max(px, (size - 1) - py);
                                        parallel = px + py;
                                        break;
                                    case DecalDirection::SOUTH_EAST:
                                        dist = std::max((size - 1) - px, (size - 1) - py);
                                        parallel = px - py;
                                        break;
                                }

                                // Cute zipper effect
                                constexpr int toothSize = 4;
                                int threshold = ((std::abs(parallel) / toothSize) % 2 == 0) ? 6 : 2;

                                if (dist < threshold) {
                                    drawPixel = true;
                                }
                            }

                            if (drawPixel) {
                                raylib::Color pixelColor = palette[0];

                                if (static_cast<DecalDirection>(col) == DecalDirection::SOLID) {
                                    // 1-pixel border on Top and Left ONLY prevents "double borders"
                                    // when tiled!
                                    if (px == 0 || py == 0) {
                                        pixelColor = palette[2];
                                    }
                                }
                                // No outline on the decals to prevent criss-crossing oddities in
                                // corners (they happen anyway, but I gave up)

                                atlasImage.DrawPixel(col * size + px, row * size + py, pixelColor);
                            }
                        }
                    }
                }
            }
        }

        raylib::Texture2D texture(atlasImage);
        texture.SetFilter(TEXTURE_FILTER_POINT);
        _atlas = std::make_shared<raylib::Texture2D>(std::move(texture));
    }

  public:
    std::shared_ptr<raylib::Texture2D> getAtlas() {
        if (!_atlas) {
            generateAtlas();
        }
        return _atlas;
    }

    int getPriority(TerrainType::Type t) const {
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
    }

    void getTileUVs(TerrainType::Type type, int variation, int col, float& u, float& v, float& uw,
                    float& vh) const {
        int b = static_cast<int>(type);
        int row = b * 4 + variation;

        // cols = 9, rows = 36
        // Small inset to prevent texture bleeding from neighboring atlas cells
        float eX = 0.1f / 288.0f;
        float eY = 0.1f / 1152.0f;

        u = (static_cast<float>(col) / 9.0f) + eX;
        v = (static_cast<float>(row) / 36.0f) + eY;
        uw = (1.0f / 9.0f) - 2.0f * eX;
        vh = (1.0f / 36.0f) - 2.0f * eY;
    }

    Tiletextures() = default;

    ~Tiletextures() = default;
};

} // namespace zappy

#endif // ZAPPY_TILETEXTURES_HPP
