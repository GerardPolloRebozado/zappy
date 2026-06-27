/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_player.cpp
*/

#include "Commands/CommandPlayerConnection.hpp"
#include "Commands/CommandPlayerDeath.hpp"
#include "Commands/CommandPlayerExpulsion.hpp"
#include "Commands/CommandPlayerInventory.hpp"
#include "Commands/CommandPlayerLevel.hpp"
#include "Commands/CommandPlayerPosition.hpp"
#include "Commands/FactoryCommands.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentShared.hpp"
#include "Components/ComponentTags.hpp"
#include "ECS/World.hpp"
#include "Systems/ParticleSystem.hpp"
#include "Systems/SimulationSystem.hpp"
#include <criterion/criterion.h>
#include <string>

using namespace zappy;

Test(CommandPlayerPositionTest, ValidUpdateExistingPosition) {
    World world;
    Entity player = world.spawn();
    world.add_component<ServerId>(player, ServerId{static_cast<int>(player.id())});

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

Test(CommandPlayerLevelTest, ValidUpdateExistingLevel) {
    World world;
    Entity player = world.spawn();
    world.add_component<ServerId>(player, ServerId{static_cast<int>(player.id())});

    world.add_component<Level>(player, Level{1});

    std::string cmdStr = std::to_string(player.id()) + " 5";

    CommandPlayerLevel cmd;
    cmd.execute(cmdStr, world);

    auto levelComp = world.get_component<Level>(player);

    cr_assert_not_null(levelComp.get());
    cr_assert_eq(levelComp->level, 5);
}

Test(CommandPlayerInventoryTest, ValidUpdateExistingInventory) {
    World world;
    Entity player = world.spawn();
    world.add_component<ServerId>(player, ServerId{static_cast<int>(player.id())});

    world.add_component<Position>(player, Position{0, 0});
    world.add_component<Inventory>(player, Inventory{1, 1, 1, 1, 1, 1, 1});

    std::string cmdStr = std::to_string(player.id()) + " 5 5 10 20 30 40 50 60 70";

    CommandPlayerInventory cmd;
    cmd.execute(cmdStr, world);

    auto pos = world.get_component<Position>(player);
    auto inv = world.get_component<Inventory>(player);

    cr_assert_not_null(pos.get());
    cr_assert_not_null(inv.get());
    cr_assert_eq(pos->x, 5);
    cr_assert_eq(pos->y, 5);
    cr_assert_eq(inv->food, 10);
    cr_assert_eq(inv->thystame, 70);
}

Test(CommandPlayerConnectionTest, ValidNewPlayer) {
    World world;
    CommandPlayerConnection cmd;

    // #n X Y O L N
    cmd.execute("50 10 20 2 1 TeamTest", world);

    auto posStorage = world.get_storage<Position>();
    bool found = false;
    for (auto const& [ent, pos] : *posStorage) {
        auto serverId = world.get_component<ServerId>(ent);
        if (serverId && serverId->id == 50) {
            cr_assert_eq(pos->x, 10);
            cr_assert_eq(pos->y, 20);
            cr_assert_not_null(world.get_component<InhabitantTag>(ent));
            cr_assert_eq(world.get_component<Level>(ent)->level, 1);
            cr_assert_eq(world.get_component<TeamName>(ent)->_team_name, "TeamTest");
            found = true;
            break;
        }
    }
    cr_assert(found);
}

Test(CommandPlayerDeathTest, ValidPlayerDeath) {
    World world;
    Entity player = world.spawn();
    world.add_component<ServerId>(player, ServerId{77});
    world.add_component<Position>(player, Position{0, 0});

    cr_assert(world.is_alive(player));

    CommandPlayerDeath cmd;
    cmd.execute("77", world);

    cr_assert(world.is_alive(player));
    cr_assert_not_null(world.get_component<EventDeath>(player).get());

    ParticleSystem ps;
    ps.update(world, 0.1f); // processes EventDeath, adds emitter, plays sound
    ps.update(world, 2.0f); // exceeds emitter maxLifetime (1.5f)

    cr_assert(!world.is_alive(player));
}

static void setupMap(World& world, int width, int height) {
    Entity mapEntity = world.spawn();
    world.add_component<Size>(mapEntity, Size{width, height});
    world.add_component<MapTag>(mapEntity, MapTag{});
}
Test(CommandPlayerExpulsionTest, FactoryCreatesPex) {
    auto& cmd = FactoryCommands::getCommand("pex");
}

Test(CommandPlayerExpulsionTest, EjectEast) {
    World world;
    setupMap(world, 10, 10);

    Entity victim = world.spawn();
    world.add_component<ServerId>(victim, ServerId{1});
    world.add_component<Position>(victim, Position{5, 5});
    world.add_component<InhabitantTag>(victim, InhabitantTag{});

    Entity ejector = world.spawn();
    world.add_component<ServerId>(ejector, ServerId{2});
    world.add_component<Position>(ejector, Position{5, 5});
    world.add_component<InhabitantTag>(ejector, InhabitantTag{});
    world.add_component<Orientation>(ejector, Orientation{Orientation::E});

    CommandPlayerExpulsion cmd;
    cmd.execute("2", world);

    SimulationSystem sys;
    sys.update(world);

    auto pos = world.get_component<Position>(victim);
    cr_assert_not_null(pos.get());
    cr_assert_eq(pos->x, 6);
    cr_assert_eq(pos->y, 5);
}

Test(CommandPlayerExpulsionTest, EjectNorthWithWrap) {
    World world;
    setupMap(world, 10, 10);

    Entity victim = world.spawn();
    world.add_component<ServerId>(victim, ServerId{10});
    world.add_component<Position>(victim, Position{3, 0});
    world.add_component<InhabitantTag>(victim, InhabitantTag{});

    Entity ejector = world.spawn();
    world.add_component<ServerId>(ejector, ServerId{11});
    world.add_component<Position>(ejector, Position{3, 0});
    world.add_component<InhabitantTag>(ejector, InhabitantTag{});
    world.add_component<Orientation>(ejector, Orientation{Orientation::N});

    CommandPlayerExpulsion cmd;
    cmd.execute("11", world);

    SimulationSystem sys;
    sys.update(world);

    auto pos = world.get_component<Position>(victim);
    cr_assert_not_null(pos.get());
    cr_assert_eq(pos->x, 3);
    cr_assert_eq(pos->y, 9);
}

Test(CommandPlayerExpulsionTest, NoCoTileInhabitant) {
    World world;
    setupMap(world, 10, 10);

    Entity alonePlayer = world.spawn();
    world.add_component<ServerId>(alonePlayer, ServerId{20});
    world.add_component<Position>(alonePlayer, Position{7, 7});
    world.add_component<InhabitantTag>(alonePlayer, InhabitantTag{});
    world.add_component<Orientation>(alonePlayer, Orientation{Orientation::S});

    CommandPlayerExpulsion cmd;
    cmd.execute("20", world);

    auto pos = world.get_component<Position>(alonePlayer);
    cr_assert_not_null(pos.get());
    cr_assert_eq(pos->x, 7);
    cr_assert_eq(pos->y, 7);
}

Test(CommandPlayerExpulsionTest, InvalidId) {
    World world;
    setupMap(world, 10, 10);

    CommandPlayerExpulsion cmd;
    cmd.execute("#999", world);
    cmd.execute("not_an_id", world);
}
