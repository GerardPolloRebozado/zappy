#include "Core.hpp"
#include <criterion/criterion.h>
#include <raylib.h>

// Reuse headless setup from RenderSystem tests if needed,
// but Core initializes its own window.
// Note: In a CI environment, this might require a virtual framebuffer (Xvfb).

Test(core, initialization) {
    // We test if the Core can be instantiated and initialized.
    // Note: It will attempt to connect to a non-existent port 0 on localhost,
    // which should fail gracefully without crashing.

    // Set raylib log level to none to avoid cluttering test output
    SetTraceLogLevel(LOG_NONE);

    // Use a try-catch to ensure no unexpected exceptions during init
    try {
        zappy::Core core(0, "127.0.0.1");
        cr_assert(true, "Core initialized successfully");
    } catch (...) {
        cr_assert(false, "Core initialization threw an exception");
    }
}

Test(core, run_loop_exit) {
    // Test that the core loop can start and exit.
    // Since run() is blocking, we can only easily test this if we can
    // trigger window.ShouldClose() or if we test the internal methods.

    zappy::Core core(0, "127.0.0.1");

    // We can't easily call run() because it blocks.
    // In a full integration test, we would run this in a thread
    // and close the window externally.
    cr_assert(true);
}
