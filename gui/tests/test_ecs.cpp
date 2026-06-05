/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_ecs.cpp
*/

#include "Components/ComponentShared.hpp"
#include "ECS/World.hpp"
#include <criterion/criterion.h>

using namespace zappy;

struct EmptyTag {};

Test(WorldTest, SpawnAndDespawn) {
    World world;
    Entity e1 = world.spawn();
    Entity e2 = world.spawn();

    cr_assert(world.is_alive(e1));
    cr_assert(world.is_alive(e2));
    cr_assert_neq(e1.id(), e2.id());

    world.despawn(e1);
    cr_assert(!world.is_alive(e1));
    cr_assert(world.is_alive(e2));
}

Test(WorldTest, ReuseIds) {
    World world;
    Entity e1 = world.spawn();
    uint32_t id1 = e1.id();
    world.despawn(e1);

    Entity e2 = world.spawn();
    cr_assert_eq(e2.id(), id1);
    cr_assert_neq(e2.generation(), e1.generation());
}

Test(WorldTest, RegisterAndGetStorage) {
    World world;
    world.register_component<Position>();
    auto storage = world.get_storage<Position>();
    cr_assert_not_null(storage.get());
}

Test(WorldTest, AddAndGetComponent) {
    World world;
    Entity e1 = world.spawn();

    world.add_component<Position>(e1, Position{10, 20});
    auto pos = world.get_component<Position>(e1);

    cr_assert_not_null(pos.get());
    cr_assert_eq(pos->x, 10);
    cr_assert_eq(pos->y, 20);
}

Test(WorldTest, MultipleComponents) {
    World world;
    Entity e1 = world.spawn();
    Entity e2 = world.spawn();

    world.add_component<Position>(e1, Position{1, 2});
    world.add_component<Position>(e2, Position{3, 4});

    auto p1 = world.get_component<Position>(e1);
    auto p2 = world.get_component<Position>(e2);

    cr_assert_eq(p1->x, 1);
    cr_assert_eq(p2->x, 3);
}

Test(WorldTest, RemoveComponent) {
    World world;
    Entity e1 = world.spawn();

    world.add_component<Position>(e1, Position{10, 20});
    world.remove_component<Position>(e1);

    auto pos = world.get_component<Position>(e1);
    cr_assert_null(pos.get());
}

Test(WorldTest, DespawnRemovesComponents) {
    World world;
    Entity e1 = world.spawn();
    world.add_component<Position>(e1, Position{10, 20});

    world.despawn(e1);

    auto storage = world.get_storage<Position>();
    cr_assert_null(storage->get(e1).get());
}

Test(WorldTest, TagsWork) {
    World world;
    Entity e1 = world.spawn();

    world.add_component<EmptyTag>(e1, EmptyTag{});
    cr_assert_not_null(world.get_component<EmptyTag>(e1).get());

    world.remove_component<EmptyTag>(e1);
    cr_assert_null(world.get_component<EmptyTag>(e1).get());
}
