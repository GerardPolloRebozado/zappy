/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** test_simulation_system.cpp
*/

#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/World.hpp"
#include "Systems/SimulationSystem.hpp"
#include <criterion/criterion.h>
#include <memory>

using namespace zappy;

Test(SimulationSystemTest, ExpulsionUpdatesPositions) {
    World world;
    SimulationSystem sys;

    Entity sizeEntity = world.spawn();
    world.add_component<Size>(sizeEntity, Size{10, 10});

    Entity executor = world.spawn();
    world.add_component<Position>(executor, Position{5, 5});
    world.add_component<Orientation>(executor, Orientation{Orientation::N});
    world.add_component<EventExpulsion>(executor, EventExpulsion{});

    Entity victim = world.spawn();
    world.add_component<InhabitantTag>(victim, InhabitantTag{});
    world.add_component<Position>(victim, Position{5, 5});

    Entity bystander = world.spawn();
    world.add_component<InhabitantTag>(bystander, InhabitantTag{});
    world.add_component<Position>(bystander, Position{6, 6});

    sys.update(world);

    auto victimPos = world.get_component<Position>(victim);
    auto bystanderPos = world.get_component<Position>(bystander);

    // Victim should be pushed North (y - 1)
    cr_assert_eq(victimPos->x, 5);
    cr_assert_eq(victimPos->y, 4);

    // Victim should receive EventExpulsed tag
    auto victimExpulsed = world.get_component<EventExpulsed>(victim);
    cr_assert_not_null(victimExpulsed);

    // Bystander should not move
    cr_assert_eq(bystanderPos->x, 6);
    cr_assert_eq(bystanderPos->y, 6);

    // EventExpulsion tag should still exist for the ParticleSystem to clean up
    auto execExpulsion = world.get_component<EventExpulsion>(executor);
    cr_assert_not_null(execExpulsion);
}

Test(SimulationSystemTest, ExpulsionWrapsMapBoundaries) {
    World world;
    SimulationSystem sys;

    Entity sizeEntity = world.spawn();
    world.add_component<Size>(sizeEntity, Size{10, 10});

    Entity executor = world.spawn();
    world.add_component<Position>(executor, Position{0, 0});
    world.add_component<Orientation>(executor, Orientation{Orientation::W});
    world.add_component<EventExpulsion>(executor, EventExpulsion{});

    Entity victim = world.spawn();
    world.add_component<InhabitantTag>(victim, InhabitantTag{});
    world.add_component<Position>(victim, Position{0, 0});

    sys.update(world);

    auto victimPos = world.get_component<Position>(victim);

    // Pushed West from 0 -> should wrap to 9
    cr_assert_eq(victimPos->x, 9);
    cr_assert_eq(victimPos->y, 0);
}

Test(SimulationSystemTest, ResourceCollectUpdatesInventories) {
    World world;
    SimulationSystem sys;

    Entity player = world.spawn();
    world.add_component<Position>(player, Position{2, 2});
    world.add_component<Inventory>(player, Inventory{0, 0, 0, 0, 0, 0, 0});
    world.add_component<EventResourceCollect>(player, EventResourceCollect{ResourceType::LINEMATE});

    Entity tile = world.spawn();
    world.add_component<Position>(tile, Position{2, 2});
    world.add_component<Inventory>(tile, Inventory{0, 5, 0, 0, 0, 0, 0});
    world.add_component<TileTag>(tile, TileTag{});

    sys.update(world);

    auto playerInv = world.get_component<Inventory>(player);
    auto tileInv = world.get_component<Inventory>(tile);

    cr_assert_eq(playerInv->linemate, 1);
    cr_assert_eq(tileInv->linemate, 4);

    // EventResourceCollect should be cleared by SimulationSystem
    auto collectEvent = world.get_component<EventResourceCollect>(player);
    cr_assert_null(collectEvent);
}

Test(SimulationSystemTest, ResourceDropUpdatesInventories) {
    World world;
    SimulationSystem sys;

    Entity player = world.spawn();
    world.add_component<Position>(player, Position{3, 3});
    world.add_component<Inventory>(player, Inventory{5, 0, 0, 0, 0, 0, 0});
    world.add_component<EventResourceDrop>(player, EventResourceDrop{ResourceType::FOOD});

    Entity tile = world.spawn();
    world.add_component<Position>(tile, Position{3, 3});
    world.add_component<Inventory>(tile, Inventory{0, 0, 0, 0, 0, 0, 0});
    world.add_component<TileTag>(tile, TileTag{});

    sys.update(world);

    auto playerInv = world.get_component<Inventory>(player);
    auto tileInv = world.get_component<Inventory>(tile);

    cr_assert_eq(playerInv->food, 4);
    cr_assert_eq(tileInv->food, 1);

    // EventResourceDrop should be cleared by SimulationSystem
    auto dropEvent = world.get_component<EventResourceDrop>(player);
    cr_assert_null(dropEvent);
}
