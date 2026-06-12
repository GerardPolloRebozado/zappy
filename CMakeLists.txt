cmake_minimum_required(VERSION 3.28)
project(zappy)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)
FetchContent_Declare(Corrosion GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git GIT_TAG v0.6.1)
FetchContent_MakeAvailable(Corrosion)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(gui)
add_subdirectory(server)
