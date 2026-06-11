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
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

namespace zappy {

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
    std::unordered_map<TerrainType::Type, std::shared_ptr<raylib::Texture2D>> _textures;

  public:
    std::shared_ptr<raylib::Texture2D> GetTileTexture(const TerrainType::Type type) {
        return _textures.at(type);
    }

    Tiletextures() {
        constexpr int size = 32;
        for (auto const& [type, palette] : _colors) {
            raylib::Image image = GenImageColor(size, size, BLANK);
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    const int randVal = std::rand() % 100;
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

                    image.DrawPixel(x, y, pixelColor);
                }
            }

            raylib::Texture2D texture(image);
            texture.SetFilter(TEXTURE_FILTER_POINT);

            _textures[type] = std::make_shared<raylib::Texture2D>(std::move(texture));
        }
    }

    ~Tiletextures() = default;
};

} // namespace zappy

#endif // ZAPPY_TILETEXTURES_HPP