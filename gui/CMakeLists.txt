cmake_minimum_required(VERSION 3.28)
project(zappy_gui)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Release or Debug" FORCE)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE) # enable wayland on linux
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE) # disable x11 on linux (switch these settings around as you please)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "6.0"
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        raylib-cpp
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
        GIT_TAG v6.0.0
)

FetchContent_MakeAvailable(raylib raylib-cpp)

set(CORE_SOURCES
        src/NetworkManager.cpp
        src/Network/TcpSocket.cpp
        src/Systems/RenderSystem.cpp
        src/Graphics/AssetManager.cpp
        include/Components/ComponentShared.hpp
        include/Components/ComponentInhabitant.hpp
        include/Components/ComponentTile.hpp
        include/Systems/RenderSystem.hpp
        include/Graphics/AssetManager.hpp
        include/Commands/ACommand.hpp
        include/Commands/FactoryCommands.hpp
        include/Commands/CommandTimeUpdate.hpp
        include/Commands/CommandTileContent.hpp
        include/Commands/CommandTeamNames.hpp
        include/Commands/CommandPlayerPosition.hpp
        include/Commands/CommandPlayerLevel.hpp
        include/Commands/CommandPlayerInventory.hpp
        include/Commands/CommandMapSize.hpp
        include/Commands/CommandMapContent.hpp
        include/Commands/FactoryCommands.cpp
        include/ECS/Entity.hpp
        include/ECS/IComponentStorage.hpp
        include/ECS/World.hpp
)

# Create a core library to avoid redundant compilation
add_library(zappy_gui_core STATIC ${CORE_SOURCES})

target_include_directories(zappy_gui_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${raylib-cpp_SOURCE_DIR}/include
)

target_link_libraries(zappy_gui_core PUBLIC raylib)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE zappy_gui_core)

if (APPLE)
  target_link_libraries(zappy_gui_core PUBLIC
            "-framework IOKit"
            "-framework Cocoa"
            "-framework OpenGL"
    )
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)

if(ipo_supported)
  set_property(TARGET zappy_gui_core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
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
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}"
        "${CMAKE_BINARY_DIR}/tests/unit_tests"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Wiping CMake cache, build files, and main executable..."
)

enable_testing()
add_subdirectory(tests)
