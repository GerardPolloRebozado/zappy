/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_player.cpp
*/

#include <criterion/criterion.h>
#include "ECS/World.hpp"
#include "Commands/CommandPlayerPosition.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentInhabitant.hpp"

using namespace zappy;

Test(CommandPlayerPositionTest, ValidUpdateExistingPosition) {
    World world;
    Entity player = world.spawn();

    InhabitantData data;
    data.id = 1;
    data.orientation = 1;

    world.add_component<InhabitantData>(player, data);
    world.add_component<Position>(player, Position{0, 0});

    CommandPlayerPosition cmd;
    cmd.execute("1 10 20 2", world);
    auto pos = world.get_component<Position>(player);
    auto inhab = world.get_component<InhabitantData>(player);

    cr_assert_not_null(pos.get());
    cr_assert_not_null(inhab.get());
    cr_assert_eq(pos->x, 10);
    cr_assert_eq(pos->y, 20);
    cr_assert_eq(inhab->orientation, 2);
}

Test(CommandPlayerPositionTest, ValidUpdateWithHashtag) {
    World world;
    Entity player = world.spawn();

    InhabitantData data;
    data.id = 4;
    data.orientation = 1;
    world.add_component<InhabitantData>(player, data);

    CommandPlayerPosition cmd;
    cmd.execute("#4 15 5 3", world);

    auto pos = world.get_component<Position>(player);
    auto inhab = world.get_component<InhabitantData>(player);

    cr_assert_not_null(pos.get());
    cr_assert_eq(pos->x, 15);
    cr_assert_eq(pos->y, 5);
    cr_assert_eq(inhab->orientation, 3);
}

Test(CommandPlayerPositionTest, ValidAddMissingPosition) {
    World world;
    Entity player = world.spawn();

    InhabitantData data;
    data.id = 2;
    data.orientation = 1;
    world.add_component<InhabitantData>(player, data);

    CommandPlayerPosition cmd;
    cmd.execute("2 7 8 4", world);

    auto pos = world.get_component<Position>(player);

    cr_assert_not_null(pos.get(), "it fails the Position");
    cr_assert_eq(pos->x, 7);
    cr_assert_eq(pos->y, 8);
}

Test(CommandPlayerPositionTest, InvalidArgumentsSyntax) {
    World world;
    Entity player = world.spawn();

    InhabitantData data;
    data.id = 3;
    data.orientation = 1;
    world.add_component<InhabitantData>(player, data);
    world.add_component<Position>(player, Position{0, 0});

    CommandPlayerPosition cmd;
    cmd.execute("3 10 invalid", world);

    auto pos = world.get_component<Position>(player);

    cr_assert_eq(pos->x, 0);
    cr_assert_eq(pos->y, 0);
}

Test(CommandPlayerPositionTest, UnknownPlayerId) {
    World world;

    CommandPlayerPosition cmd;
    cmd.execute("99 10 10 1", world);

    cr_assert(true);
}