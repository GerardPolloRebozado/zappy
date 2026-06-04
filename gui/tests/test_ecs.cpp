#include "Components/ComponentShared.hpp"
#include "ECS/World.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(ECSTest, SpawnEntity) {
    World world;
    Entity e1 = world.spawn();
    Entity e2 = world.spawn();

    cr_assert(e1.id() != e2.id());
    cr_assert(world.is_alive(e1));
    cr_assert(world.is_alive(e2));
}

Test(ECSTest, DespawnEntity) {
    World world;
    Entity e1 = world.spawn();

    cr_assert(world.is_alive(e1));
    world.despawn(e1);
    cr_assert(!world.is_alive(e1));
}

Test(ECSTest, EntityGeneration) {
    World world;
    Entity e1 = world.spawn();
    uint32_t id = e1.id();
    uint32_t gen = e1.generation();

    world.despawn(e1);
    Entity e2 = world.spawn();

    cr_assert_eq(e2.id(), id);
    cr_assert_eq(e2.generation(), gen + 1);
    cr_assert(!world.is_alive(e1));
    cr_assert(world.is_alive(e2));
}

Test(ECSTest, AddGetComponent) {
    World world;
    Entity e1 = world.spawn();

    Position pos = {10, 20};
    world.add_component<Position>(e1, pos);

    auto retrievedPos = world.get_component<Position>(e1);
    cr_assert_not_null(retrievedPos);
    cr_assert_eq(retrievedPos->x, 10);
    cr_assert_eq(retrievedPos->y, 20);
}

Test(ECSTest, GetNonExistentComponent) {
    World world;
    Entity e1 = world.spawn();

    auto retrievedPos = world.get_component<Position>(e1);
    cr_assert_null(retrievedPos);
}

Test(ECSTest, RemoveComponent) {
    World world;
    Entity e1 = world.spawn();

    world.add_component<Position>(e1, {10, 20});
    cr_assert_not_null(world.get_component<Position>(e1));

    world.remove_component<Position>(e1);
    cr_assert_null(world.get_component<Position>(e1));
}

Test(ECSTest, StorageIteration) {
    World world;
    Entity e1 = world.spawn();
    Entity e2 = world.spawn();

    world.add_component<Position>(e1, {1, 2});
    world.add_component<Position>(e2, {3, 4});

    auto storage = world.get_storage<Position>();
    cr_assert_not_null(storage);

    int count = 0;
    for (auto const& [ent, pos] : *storage) {
        if (ent == e1) {
            cr_assert_eq(pos->x, 1);
            cr_assert_eq(pos->y, 2);
        } else if (ent == e2) {
            cr_assert_eq(pos->x, 3);
            cr_assert_eq(pos->y, 4);
        } else {
            cr_assert_fail("Unexpected entity in storage");
        }
        count++;
    }
    cr_assert_eq(count, 2);
}

Test(ECSTest, ComponentRegistration) {
    World world;
    world.register_component<Position>();
    auto storage = world.get_storage<Position>();
    cr_assert_not_null(storage);
}

Test(ECSTest, DespawnCleanupComponents) {
    World world;
    Entity e1 = world.spawn();
    world.add_component<Position>(e1, {10, 20});

    cr_assert_not_null(world.get_component<Position>(e1));
    world.despawn(e1);

    // After despawn, components should be removed from storage
    cr_assert_null(world.get_component<Position>(e1));
}

struct EmptyTag {};

Test(ECSTest, EmptyComponent) {
    World world;
    const Entity e1 = world.spawn();
    world.add_component<EmptyTag>(e1, EmptyTag{});
    cr_assert_not_null(world.get_storage<EmptyTag>());
    cr_assert_not_null(world.get_component<EmptyTag>(e1));
}
