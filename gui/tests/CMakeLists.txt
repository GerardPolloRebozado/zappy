cmake_minimum_required(VERSION 3.28)

set(TEST_SOURCES
    test_network.cpp
    ../src/NetworkManager.cpp
    ../src/Network/TcpSocket.cpp
)

add_executable(unit_tests ${TEST_SOURCES})

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(unit_tests PRIVATE criterion)

add_test(NAME unit_tests COMMAND unit_tests)
