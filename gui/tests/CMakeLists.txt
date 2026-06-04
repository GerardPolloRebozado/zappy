cmake_minimum_required(VERSION 3.28)

set(TEST_SOURCES
    test_ecs.cpp
    test_network.cpp
    test_render_system.cpp
    ../src/NetworkManager.cpp
    ../src/Network/TcpSocket.cpp
    ../src/Systems/RenderSystem.cpp
    ../include/Commands/FactoryCommands.cpp
)

add_executable(unit_tests ${TEST_SOURCES})

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${raylib-cpp_SOURCE_DIR}/include
)

target_link_libraries(unit_tests PRIVATE criterion raylib)

add_test(NAME unit_tests COMMAND unit_tests)
