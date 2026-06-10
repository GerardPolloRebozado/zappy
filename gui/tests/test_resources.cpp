/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_resources.cpp
*/

#include "Commands/CommandResourceCollect.hpp"
#include "Commands/CommandResourceDrop.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/World.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(CommandResourceTest, CollectResource) {
    World world;
    Entity player = world.spawn();
    world.add_component<ServerId>(player, ServerId{100});
    Entity tile = world.spawn();

    world.add_component<Position>(player, Position{2, 3});
    world.add_component<Inventory>(player, Inventory{10, 0, 0, 0, 0, 0, 0});

    world.add_component<Position>(tile, Position{2, 3});
    world.add_component<Inventory>(tile, Inventory{5, 5, 5, 5, 5, 5, 5});
    world.add_component<TileTag>(tile, TileTag{});

    CommandResourceCollect cmd;
    cmd.execute("100 1", world); // Collect linemate (1)

    auto playerInv = world.get_component<Inventory>(player);
    auto tileInv = world.get_component<Inventory>(tile);

    cr_assert_not_null(playerInv);
    cr_assert_not_null(tileInv);
    cr_assert_eq(playerInv->linemate, 1);
    cr_assert_eq(tileInv->linemate, 4);
}

Test(CommandResourceTest, DropResource) {
    World world;
    Entity player = world.spawn();
    world.add_component<ServerId>(player, ServerId{200});
    Entity tile = world.spawn();

    world.add_component<Position>(player, Position{1, 1});
    world.add_component<Inventory>(player, Inventory{10, 10, 10, 10, 10, 10, 10});

    world.add_component<Position>(tile, Position{1, 1});
    world.add_component<Inventory>(tile, Inventory{0, 0, 0, 0, 0, 0, 0});
    world.add_component<TileTag>(tile, TileTag{});

    CommandResourceDrop cmd;
    cmd.execute("200 0", world); // Drop food (0)

    auto playerInv = world.get_component<Inventory>(player);
    auto tileInv = world.get_component<Inventory>(tile);

    cr_assert_not_null(playerInv);
    cr_assert_not_null(tileInv);
    cr_assert_eq(playerInv->food, 9);
    cr_assert_eq(tileInv->food, 1);
}
