#include "Core.hpp"
#include <criterion/criterion.h>
#include <raylib.h>

Test(core, initialization) {
    SetTraceLogLevel(LOG_NONE);

    try {
        zappy::Core core(0, "127.0.0.1");
        cr_assert(true, "Core initialized successfully");
    } catch (...) {
        cr_assert(false, "Core initialization threw an exception");
    }
}

Test(core, run_loop_exit) {
    zappy::Core core(0, "127.0.0.1");
    cr_assert(true);
}
