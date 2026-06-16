/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_team.cpp
*/

#include "Commands/CommandTeamNames.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/World.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(CommandTeamNamesTest, AddNewTeam) {
    World world;
    CommandTeamNames cmd;

    cmd.execute("Team1", world);

    auto storage = world.get_storage<TeamName>();
    cr_assert_not_null(storage);

    bool found = false;
    for (auto const& [ent, name] : *storage) {
        if (name->_team_name == "Team1") {
            found = true;
            cr_assert(world.get_component<TeamTag>(ent) != nullptr);
        }
    }
    cr_assert(found);
}

Test(CommandTeamNamesTest, DoNotAddDuplicateTeam) {
    World world;
    CommandTeamNames cmd;

    cmd.execute("Team1", world);
    cmd.execute("Team1", world);

    auto storage = world.get_storage<TeamName>();
    int count = 0;
    for (auto const& [ent, name] : *storage) {
        if (name->_team_name == "Team1") {
            count++;
        }
    }
    cr_assert_eq(count, 1);
}

Test(CommandTeamNamesTest, TrimWhitespace) {
    World world;
    CommandTeamNames cmd;

    cmd.execute("  TeamWhitespace  ", world);

    auto storage = world.get_storage<TeamName>();
    bool found = false;
    for (auto const& [ent, name] : *storage) {
        if (name->_team_name == "TeamWhitespace") {
            found = true;
        }
    }
    cr_assert(found);
}
