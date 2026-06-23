/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_incantation.cpp
*/

#include "Commands/CommandIncantationEnd.hpp"
#include "Commands/CommandIncantationStart.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "ECS/World.hpp"
#include "Systems/ParticleSystem.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(CommandIncantationTest, StartIncantation) {
    World world;
    CommandIncantationStart cmd;

    // X Y L #n #n ...
    cmd.execute("10 10 2 1 2 3", world);

    // Currently only prints, but we can verify it doesn't crash
    cr_assert(true);
}

Test(CommandIncantationTest, EndIncantation) {
    World world;
    CommandIncantationEnd cmd;

    // X Y R
    cmd.execute("10 10 1", world);

    cr_assert(true);
}
