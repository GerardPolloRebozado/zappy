cmake_minimum_required(VERSION 3.28)
project(zappy_gui)

set(ZAPPY_LOG_LEVEL 3 CACHE STRING "1=ERROR, 2=DEBUG, 3=INFO")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Release or Debug" FORCE)
endif ()

if (APPLE)
  set(PLATFORM "SDL" CACHE STRING "Platform target" FORCE)
  set(OPENGL_VERSION "3.3" CACHE STRING "OpenGL version to use" FORCE)
endif ()

if (DEFINED ENV{IM_STUPID} AND "$ENV{IM_STUPID}" STREQUAL "ON")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main
)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)

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

FetchContent_MakeAvailable(SDL3 raylib raylib-cpp)

set(CORE_SOURCES
        src/NetworkManager.cpp
        src/Network/TcpSocket.cpp
        src/Systems/RenderSystem.cpp
        src/Systems/ParticleSystem.cpp
        src/Graphics/AssetManager.cpp
        src/Graphics/VoxelBatcher.cpp
        src/UI/AUIComponent.cpp
        src/UI/UIManager.cpp
        src/UI/UIButton.cpp
        src/UI/UIInput.cpp
        src/UI/UIPanel.cpp
        src/UI/UIText.cpp
        src/UI/UIHudPanel.cpp
        src/UI/UIScoreboardPanel.cpp
        src/Core.cpp
        include/Components/ComponentShared.hpp
        include/Components/ComponentInhabitant.hpp
        include/Components/ComponentTile.hpp
        include/UI/IUIComponent.hpp
        include/UI/AUIComponent.hpp
        include/UI/UIEvent.hpp
        include/UI/UIManager.hpp
        include/UI/UIButton.hpp
        include/UI/UIInput.hpp
        include/UI/UIPanel.hpp
        include/UI/UIText.hpp
        include/UI/UIHudPanel.hpp
        include/UI/UIScoreboardPanel.hpp
        include/Systems/RenderSystem.hpp
        include/Graphics/AssetManager.hpp
        include/Core.hpp
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

add_library(zappy_gui_core STATIC ${CORE_SOURCES})

target_include_directories(zappy_gui_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${raylib-cpp_SOURCE_DIR}/include
        ${SDL3_SOURCE_DIR}/include
)

target_link_libraries(zappy_gui_core PUBLIC raylib SDL3::SDL3)
target_compile_definitions(zappy_gui_core PRIVATE ZAPPY_LOG_LEVEL=${ZAPPY_LOG_LEVEL})

if(APPLE)
  target_compile_definitions(zappy_gui_core PUBLIC GRAPHICS_API_OPENGL_33)
endif()

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE zappy_gui_core)
target_compile_definitions(${PROJECT_NAME} PRIVATE ZAPPY_LOG_LEVEL=${ZAPPY_LOG_LEVEL})

if (APPLE)
  target_link_libraries(zappy_gui_core PUBLIC
            "-framework IOKit"
            "-framework Cocoa"
            "-framework CoreFoundation"
            "-framework CoreVideo"
            "-framework QuartzCore"
            "-framework Metal"
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
)

enable_testing()
add_subdirectory(tests)
