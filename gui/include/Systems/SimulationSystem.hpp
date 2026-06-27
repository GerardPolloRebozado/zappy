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

/**
 * @class SimulationSystem
 * @brief Handles logic execution independent of the network or rendering
 *
 * The SimulationSystem puts together the Zappy server's "event-driven" packet architecture
 * with the ECS by consuming event tags (like EventExpulsion, EventResourceCollect)
 * and modifying the corresponding simulation components (Position, Inventory, etc.).
 * It serves  as the "Physics/Logic" engine of the client, making sure
 * state updates happen deterministically on the client side without needing the server
 * to broadcast follow-up state packets for every interaction
 */
class SimulationSystem {
  public:
    SimulationSystem() = default;
    ~SimulationSystem() = default;

    /**
     * @brief Updates the simulation, executing logic for all queued event tags
     * @param world The ECS world containing all components
     */
    void update(World& world);

  private:
    /**
     * @brief Processes EventExpulsion tags, moving players who share the tile forward
     * @param world The ECS world
     */
    void _handleExpulsions(World& world);

    /**
     * @brief Processes EventResourceCollect tags, updating inventories and spawning visual effects
     * @param world The ECS world
     */
    void _handleResourceCollect(World& world);

    /**
     * @brief Processes EventResourceDrop tags, updating inventories and spawning visual effects.
     * @param world The ECS world.
     */
    void _handleResourceDrop(World& world);

    /**
     * @brief Moves a position forward based on orientation, wrapping around the map boundaries.
     * @param pos The position to modify.
     * @param direction The direction to move.
     * @param mapWidth The width of the map.
     * @param mapHeight The height of the map.
     */
    void _moveForward(const std::shared_ptr<Position>& pos, Orientation::Direction direction,
                      int mapWidth, int mapHeight);

    /**
     * @brief Modifies an inventory by adding or subtracting a specific resource.
     * @param inv The inventory to modify.
     * @param resourceId The resource to adjust.
     * @param delta The amount to adjust by (positive for adding, negative for removing).
     */
    void _updateInventory(Inventory* inv, ResourceType resourceId, int delta);
};

} // namespace zappy

#endif
