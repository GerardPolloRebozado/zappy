/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** SimulationSystem.hpp
*/

#ifndef ZAPPY_GUI_SIMULATIONSYSTEM_HPP
#define ZAPPY_GUI_SIMULATIONSYSTEM_HPP

#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/World.hpp"

namespace zappy {

class SimulationSystem {
  public:
    SimulationSystem() = default;
    ~SimulationSystem() = default;

    void update(World& world);

  private:
    void _handleExpulsions(World& world);
    void _handleResourceCollect(World& world);
    void _handleResourceDrop(World& world);

    void _moveForward(const std::shared_ptr<Position>& pos, Orientation::Direction direction,
                      int mapWidth, int mapHeight);
    void _updateInventory(Inventory* inv, ResourceType resourceId, int delta);
};

} // namespace zappy

#endif
