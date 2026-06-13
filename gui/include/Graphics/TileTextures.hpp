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
                            int local_nx = px + var * size;
                            int local_ny = py + var * size;

                            bool drawPixel = false;

                            if (col == 0) {
                                // Solid base
                                drawPixel = true;
                            } else {
                                // Bleed decal
                                int dist = 999;
                                if (col == 1) {
                                    dist = py; // North (y - 1)
                                }
                                if (col == 2) {
                                    dist = (size - 1) - py; // South (y + 1)
                                }
                                if (col == 3) {
                                    dist = (size - 1) - px; // East (x + 1)
                                }
                                if (col == 4) {
                                    dist = px; // West (x - 1)
                                }
                                if (col == 5) {
                                    dist = std::max(px, py); // NW
                                }
                                if (col == 6) {
                                    dist = std::max((size - 1) - px, py); // NE
                                }
                                if (col == 7) {
                                    dist = std::max(px, (size - 1) - py); // SW
                                }
                                if (col == 8) {
                                    dist = std::max((size - 1) - px, (size - 1) - py); // SE
                                }

                                uint32_t noise = fast_hash(local_nx, local_ny);
                                if (dist < 3 + (int)(noise % 6)) {
                                    drawPixel = true;
                                }
                            }

                            if (drawPixel) {
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
        u = static_cast<float>(col) / 9.0f;
        v = static_cast<float>(row) / 36.0f;
        uw = 1.0f / 9.0f;
        vh = 1.0f / 36.0f;
    }

    Tiletextures() = default;

    ~Tiletextures() = default;
};

} // namespace zappy

#endif // ZAPPY_TILETEXTURES_HPP
