/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_egg.cpp
*/

#include "Commands/CommandEggConnection.hpp"
#include "Commands/CommandEggDeath.hpp"
#include "Commands/CommandEggLayed.hpp"
#include "Commands/CommandPlayerFork.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/World.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(CommandEggTest, EggLayed) {
    World world;
    CommandEggLayed cmd;

    // #e #n X Y
    cmd.execute("42 1 5 5", world);

    auto eggStorage = world.get_storage<Egg>();
    cr_assert_not_null(eggStorage);

    bool found = false;
    for (auto const& [ent, egg] : *eggStorage) {
        if (egg->id == 42) {
            auto pos = world.get_component<Position>(ent);
            cr_assert_eq(pos->x, 5);
            cr_assert_eq(pos->y, 5);
            cr_assert_not_null(world.get_component<EggTag>(ent));
            found = true;
            break;
        }
    }
    cr_assert(found);
}

Test(CommandEggTest, EggConnection) {
    World world;
    Entity eggEnt = world.spawn();
    world.add_component<Egg>(eggEnt, Egg{10});

    cr_assert(world.is_alive(eggEnt));

    CommandEggConnection cmd;
    cmd.execute("10", world);

    cr_assert(!world.is_alive(eggEnt));
}

Test(CommandEggTest, EggDeath) {
    World world;
    Entity eggEnt = world.spawn();
    world.add_component<Egg>(eggEnt, Egg{20});

    cr_assert(world.is_alive(eggEnt));

    CommandEggDeath cmd;
    cmd.execute("20", world);

    cr_assert(!world.is_alive(eggEnt));
}

Test(CommandEggTest, PlayerFork) {
    World world;
    CommandPlayerFork cmd;

    cmd.execute("1", world);
    cr_assert(true);
}
