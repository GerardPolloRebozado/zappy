/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_player.cpp
*/

#include <criterion/criterion.h>
#include <string>
#include "ECS/World.hpp"
#include "Commands/CommandPlayerPosition.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentInhabitant.hpp" //

using namespace zappy;

Test(CommandPlayerPositionTest, ValidUpdateExistingPosition) {
    World world;
    Entity player = world.spawn();

    world.add_component<Orientation>(player, Orientation{Orientation::N}); // N = 1
    world.add_component<Position>(player, Position{0, 0});

    std::string cmdStr = std::to_string(player.id()) + " 10 20 2";

    CommandPlayerPosition cmd;
    cmd.execute(cmdStr, world);

    auto pos = world.get_component<Position>(player);
    auto orient = world.get_component<Orientation>(player);

    cr_assert_not_null(pos.get());
    cr_assert_not_null(orient.get());
    cr_assert_eq(pos->x, 10);
    cr_assert_eq(pos->y, 20);
    cr_assert_eq(orient->current_direction, Orientation::E); // 2 = East
}

Test(CommandPlayerPositionTest, ValidUpdateWithHashtag) {
    World world;
    Entity player = world.spawn();

    world.add_component<Orientation>(player, Orientation{Orientation::N});
    world.add_component<Position>(player, Position{0, 0});

    std::string cmdStr = "#" + std::to_string(player.id()) + " 15 5 3";

    CommandPlayerPosition cmd;
    cmd.execute(cmdStr, world);

    auto pos = world.get_component<Position>(player);
    auto orient = world.get_component<Orientation>(player);

    cr_assert_not_null(pos.get());
    cr_assert_eq(pos->x, 15);
    cr_assert_eq(pos->y, 5);
    cr_assert_eq(orient->current_direction, Orientation::S); // 3 = South
}

Test(CommandPlayerPositionTest, MissingPositionFailsGracefully) {
    World world;
    Entity player = world.spawn();

    world.add_component<Orientation>(player, Orientation{Orientation::N});

    std::string cmdStr = std::to_string(player.id()) + " 7 8 4";

    CommandPlayerPosition cmd;
    cmd.execute(cmdStr, world);

    auto pos = world.get_component<Position>(player);

    cr_assert_null(pos.get(), "it fails the Position");
}

Test(CommandPlayerPositionTest, InvalidArgumentsSyntax) {
    World world;
    Entity player = world.spawn();

    world.add_component<Orientation>(player, Orientation{Orientation::N});
    world.add_component<Position>(player, Position{0, 0});

    std::string cmdStr = std::to_string(player.id()) + " 10 invalid";

    CommandPlayerPosition cmd;
    cmd.execute(cmdStr, world);

    auto pos = world.get_component<Position>(player);

    cr_assert_eq(pos->x, 0);
    cr_assert_eq(pos->y, 0);
}

Test(CommandPlayerPositionTest, UnknownPlayerId) {
    World world;

    CommandPlayerPosition cmd;
    cmd.execute("9999 10 10 1", world);

    cr_assert(true);
}