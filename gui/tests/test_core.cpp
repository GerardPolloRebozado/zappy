#include "Core.hpp"
#include "Graphics/AssetManager.hpp"
#include <criterion/criterion.h>
#include <raylib.h>

Test(core, initialization) {
    SetTraceLogLevel(LOG_NONE);

    try {
        zappy::Core core(0, "127.0.0.1");
        cr_assert(true, "Core initialized successfully");
        // Must unload assets while the Window (owned by core) is still open
        zappy::AssetManager::getInstance().unloadAll();
    } catch (...) {
        cr_assert(false, "Core initialization threw an exception");
    }
}

Test(core, run_loop_exit) {
    zappy::Core core(0, "127.0.0.1");
    cr_assert(true);
    // Must unload assets while the Window (owned by core) is still open
    zappy::AssetManager::getInstance().unloadAll();
}
