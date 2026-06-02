/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** Register.hpp
*/
#ifndef ZAPPY_GUI_REGISTER_HPP
#define ZAPPY_GUI_REGISTER_HPP
#include <unordered_map>
#include <queue>

#include "Components/ComponentBot.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTile.hpp"
#include "Components/ComponentTags.hpp"


namespace zappy {
    class Register {
    public:
        std::unordered_map<int, Position> _positions;
        std::unordered_map<int, Inventory> _inventories;
        std::unordered_map<int, Renderable3D> _renderables;
        std::unordered_map<int, TerrainModifiers> _terrainModifiers;
        std::unordered_map<int, TerrainType> _terrainTypes;
        std::unordered_map<int, BotData> _bots;
        std::unordered_map<int, Race> _races;
        std::unordered_map<int, Size> _sizes;

        std::unordered_map<int, TileTag> _tileTags;
        std::unordered_map<int, InhabitantTag> _botTags;
        std::unordered_map<int, EggTag> _eggTags;
        std::unordered_map<int, EggTag> _mapTags;

        int createEntity() {
            if (!_deadEntities.empty()) {
                int id = _deadEntities.front();
                _deadEntities.pop();
                return id;
            }
            return _nextEntityId++;
        }

        void destroyEntity(int entity) {
            _positions.erase(entity);
            _inventories.erase(entity);
            _renderables.erase(entity);
            _terrainModifiers.erase(entity);
            _terrainTypes.erase(entity);
            _bots.erase(entity);
            _races.erase(entity);
            _sizes.erase(entity);

            _tileTags.erase(entity);
            _botTags.erase(entity);
            _eggTags.erase(entity);
            _mapTags.erase(entity);


            _deadEntities.push(entity);
        }

    private:
        int _nextEntityId = 0;
        std::queue<int> _deadEntities;
    };
} // zappy

#endif //ZAPPY_GUI_REGISTER_HPP
