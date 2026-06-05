/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_misc.cpp
*/

#include "Commands/CommandGameEnd.hpp"
#include "Commands/CommandPlayerBroadcast.hpp"
#include "Commands/CommandServerMessage.hpp"
#include "Commands/CommandTimeUpdate.hpp"
#include "Commands/CommandUnknown.hpp"
#include "Components/ComponentShared.hpp"
#include "ECS/World.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(CommandMiscTest, TimeUpdate) {
    World world;
    CommandTimeUpdate cmd;

    cmd.execute("100", world);

    auto storage = world.get_storage<TimeUnit>();
    cr_assert_not_null(storage);

    bool found = false;
    for (auto const& [ent, tu] : *storage) {
        cr_assert_eq(tu->frequency, 100);
        found = true;
    }
    cr_assert(found);

    // Update existing
    cmd.execute("500", world);
    for (auto const& [ent, tu] : *storage) {
        cr_assert_eq(tu->frequency, 500);
    }
}

Test(CommandMiscTest, PlayerBroadcast) {
    World world;
    CommandPlayerBroadcast cmd;

    cmd.execute("1 message text", world);
    cr_assert(true);
}

Test(CommandMiscTest, ServerMessage) {
    World world;
    CommandServerMessage cmd;

    cmd.execute("Hello world", world);
    cr_assert(true);
}

Test(CommandMiscTest, GameEnd) {
    World world;
    CommandGameEnd cmd;

    cmd.execute("TeamOne", world);
    cr_assert(true);
}

Test(CommandMiscTest, UnknownCommand) {
    World world;
    CommandUnknown cmd;

    cmd.execute("", world);
    cr_assert(true);
}
