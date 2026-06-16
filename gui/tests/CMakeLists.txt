cmake_minimum_required(VERSION 3.28)

set(TEST_SOURCES
    test_ecs.cpp
    test_network.cpp
    test_render_system.cpp
    test_core.cpp
    test_player.cpp
    test_team.cpp
    test_resources.cpp
    test_incantation.cpp
    test_egg.cpp
    test_misc.cpp
    test_ui.cpp
    test_particle_system.cpp
)

add_executable(unit_tests ${TEST_SOURCES})

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${raylib-cpp_SOURCE_DIR}/include
)

target_link_libraries(unit_tests PRIVATE criterion zappy_gui_core)

target_compile_definitions(unit_tests PRIVATE ZAPPY_LOG_LEVEL=0)

add_test(NAME unit_tests COMMAND unit_tests)
