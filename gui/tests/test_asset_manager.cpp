/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** test_asset_manager.cpp
*/

#include "Graphics/AssetManager.hpp"
#include <criterion/criterion.h>
#include <raylib.h>

// initialize a headless window for testing GPU-dependent logic
static void setup_headless_gui() {
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(100, 100, "Headless Test");
}

static void teardown_headless_gui() { CloseWindow(); }

Test(asset_manager, singleton_instance) {
    zappy::AssetManager& instance1 = zappy::AssetManager::getInstance();
    zappy::AssetManager& instance2 = zappy::AssetManager::getInstance();

    cr_assert_eq(&instance1, &instance2, "AssetManager should be a singleton");
}

Test(asset_manager, get_nonexistent_assets, .init = setup_headless_gui,
     .fini = teardown_headless_gui) {
    auto& am = zappy::AssetManager::getInstance();

    // These should not crash and return "empty" assets
    auto& model = am.getModel("nonexistent_model");
    auto& texture = am.getTexture("nonexistent_texture");
    auto& shader = am.getShader("nonexistent_shader");

    cr_expect_eq(texture.id, 0, "Nonexistent texture should have id 0");
    // Shaders have a default ID in raylib-cpp, so we just check it doesn't crash
    cr_assert(true, "Getting nonexistent assets should not crash");
}

Test(asset_manager, load_all, .init = setup_headless_gui, .fini = teardown_headless_gui) {
    auto& am = zappy::AssetManager::getInstance();

    // loadAll should run without crashing even if files are missing (raylib logs errors)
    am.loadAll();

    cr_assert(true, "loadAll() should run without crashing");
}
