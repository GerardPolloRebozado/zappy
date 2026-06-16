/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** test_particle_system.cpp
*/

#include "Components/ComponentParticleEmitter.hpp"
#include "ECS/World.hpp"
#include "Systems/ParticleSystem.hpp"
#include <criterion/criterion.h>

using namespace zappy;

Test(ParticleSystemTest, SpawnsParticles) {
    World world;
    ParticleSystem ps;

    Entity e = world.spawn();
    ComponentParticleEmitter emitter;
    emitter.isPlaying = true;
    emitter.emitRate = 10.0f; // 10 particles per second
    emitter.loop = true;
    emitter.minLifetime = 5.0f; // High lifetime
    emitter.maxLifetime = 5.0f;
    world.add_component(e, emitter);

    ps.update(world, 1.0f); // 1 second passes

    auto storage = world.get_storage<ComponentParticleEmitter>();
    cr_assert(storage != nullptr);
    auto comp = storage->get(e);
    cr_assert(comp != nullptr);
    cr_assert_eq(comp->particles.size(), 10,
                 "Expected 10 particles spawned in 1 second at 10 emitRate");
}

Test(ParticleSystemTest, DespawnsEntityWhenFinished) {
    World world;
    ParticleSystem ps;

    Entity e = world.spawn();
    ComponentParticleEmitter emitter;
    emitter.isPlaying = true;
    emitter.loop = false;
    emitter.duration = 0.5f;
    emitter.emitRate = 100.0f;
    emitter.maxLifetime = 0.1f;
    world.add_component(e, emitter);

    // After 0.5s, it finishes duration. Particles are still alive because their lifetime is 0.1s
    // max.
    ps.update(world, 0.5f);
    cr_assert(world.is_alive(e), "Entity should still be alive during duration");

    // After another 0.2s, particles should expire and entity should despawn.
    ps.update(world, 0.2f);
    cr_assert(!world.is_alive(e), "Entity should be despawned after particles expire");
}

Test(ParticleSystemTest, ColorPaletteUsage) {
    World world;
    ParticleSystem ps;

    Entity e = world.spawn();
    ComponentParticleEmitter emitter;
    emitter.isPlaying = true;
    emitter.emitRate = 10.0f;
    emitter.loop = true;
    emitter.minLifetime = 5.0f; // High lifetime so they don't die instantly
    emitter.maxLifetime = 5.0f;
    emitter.colorPalette = {raylib::Color::Red(), raylib::Color::Blue()};
    world.add_component(e, emitter);

    ps.update(world, 1.0f); // Spawns 10 particles

    auto comp = world.get_component<ComponentParticleEmitter>(e);
    cr_assert(comp != nullptr);
    cr_assert(comp->particles.size() > 0, "Expected particles to spawn");

    for (const auto& p : comp->particles) {
        bool isRed =
            (p.startColor.r == raylib::Color::Red().r && p.startColor.g == raylib::Color::Red().g &&
             p.startColor.b == raylib::Color::Red().b);
        bool isBlue = (p.startColor.r == raylib::Color::Blue().r &&
                       p.startColor.g == raylib::Color::Blue().g &&
                       p.startColor.b == raylib::Color::Blue().b);
        cr_assert(isRed || isBlue, "Particle start color must be from the palette");
        cr_assert_eq(p.endColor.a, 0, "End color alpha must be 0 for palette colored particles");
    }
}
