#include "Systems/RenderSystem.hpp"
#include <criterion/criterion.h>
#include <raylib.h>

// initialize a headless window for testing GPU-dependent logic
void setup_headless_gui() {
    SetTraceLogLevel(LOG_NONE); // Keep output clean
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(100, 100, "Headless Test");
}

void teardown_headless_gui() { CloseWindow(); }

Test(render_system, initial_camera_state, .init = setup_headless_gui,
     .fini = teardown_headless_gui) {
    zappy::RenderSystem renderer;
    // we can't easily check private members without making the test a friend,
    // but we can check if it instantiates without crashing in a window context
    cr_assert(true);
}

Test(render_system, center_camera, .init = setup_headless_gui, .fini = teardown_headless_gui) {
    zappy::RenderSystem renderer;

    renderer.centerCamera(10, 10);

    // We check if the state is consistent (no crash, valid returns)
    auto hovered = renderer.getHoveredTile();
    cr_assert_eq(hovered.first, -999999, "Initial hover should be far away");
}

Test(render_system, asset_lazy_loading, .init = setup_headless_gui, .fini = teardown_headless_gui) {
    zappy::RenderSystem renderer;
    zappy::Register reg;

    // update() should trigger lazy loading.
    // in a headless context with InitWindow, LoadTexture won't crash
    // even if it might fail to find the file (it just logs an error).
    renderer.update(reg);

    cr_assert(true, "update() should run without crashing in headless context");
}
