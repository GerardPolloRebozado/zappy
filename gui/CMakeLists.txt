cmake_minimum_required(VERSION 3.28)
project(zappy_gui)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Release or Debug" FORCE)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(SOURCES
        src/main.cpp
)

# 1. Create the target first
add_executable(zappy_gui ${SOURCES})

# Raylib FetchContent
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE) # enable wayland on linux
set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE) # disable x11 on linux (switch these settings around as you please)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "6.0"
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

# 2. Link libraries using the KEYWORD signature (PRIVATE) for EVERYTHING
if (APPLE)
    target_link_libraries(zappy_gui PRIVATE
            "-framework IOKit"
            "-framework Cocoa"
            "-framework OpenGL"
    )
endif()

target_link_libraries(zappy_gui PRIVATE raylib)

# 3. Include directories
target_include_directories(zappy_gui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(ipo_supported)
    set_property(TARGET zappy_gui PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_custom_target(fclean
        COMMAND ${CMAKE_COMMAND} -E rm -rf
        "${CMAKE_BINARY_DIR}/CMakeCache.txt"
        "${CMAKE_BINARY_DIR}/CMakeFiles"
        "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        "${CMAKE_BINARY_DIR}/Makefile"
        "${CMAKE_BINARY_DIR}/build.ninja"
        "${CMAKE_BINARY_DIR}/.ninja_deps"
        "${CMAKE_BINARY_DIR}/.ninja_log"
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_BINARY_DIR}/zappy_gui"
        "${CMAKE_BINARY_DIR}/tests/unit_tests"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Wiping CMake cache, build files, and main executable..."
)

enable_testing()
#add_subdirectory(tests) remove later when tests are added