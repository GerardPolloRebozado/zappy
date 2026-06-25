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
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2" CACHE STRING "Graphics API to use" FORCE)
endif ()

if (DEFINED ENV{IM_STUPID} AND "$ENV{IM_STUPID}" STREQUAL "ON")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

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
        src/Systems/Render/RenderSystem.cpp
        src/Systems/Render/RenderSystemBatches.cpp
        src/Systems/Render/RenderSystemTerrain.cpp
        src/Systems/Render/RenderSystemEntities.cpp
        src/Systems/Render/RenderSystemResources.cpp
        src/Systems/Render/RenderSystemEffects.cpp
        src/Systems/Particles/ParticleSystem.cpp
        src/Systems/Particles/ParticleSystemHandleEggs.cpp
        src/Systems/Particles/ParticleSystemHandleDeaths.cpp
        src/Systems/Particles/ParticleSystemHandleIncantationStart.cpp
        src/Systems/Particles/ParticleSystemHandleIncantationEnd.cpp
        src/Systems/Particles/ParticleSystemHandleJump.cpp
        src/Systems/Particles/ParticleSystemHandleBroadcast.cpp
        src/Systems/Particles/ParticleSystemEmitters.cpp
        src/Systems/Animations/AnimationSystem.cpp
        src/Systems/Animations/AnimationSystemMovement2D.cpp
        src/Systems/Animations/AnimationSystemMovement3D.cpp
        src/Systems/Animations/AnimationSystemAnimations.cpp
        src/Systems/BackgroundSystem.cpp
        src/Systems/MusicSystem.cpp
        src/Graphics/AssetManager/AssetManager.cpp
        src/Graphics/AssetManager/AssetManagerModels.cpp
        src/Graphics/AssetManager/AssetManagerTextures.cpp
        src/Graphics/AssetManager/AssetManagerShaders.cpp
        src/Graphics/AssetManager/AssetManagerAnimations.cpp
        src/Graphics/AssetManager/AssetManagerAudio.cpp
        src/Graphics/VoxelBatcher.cpp
        src/UI/AUIComponent.cpp
        src/UI/UIManager.cpp
        src/UI/UIChatPanel.cpp
        src/UI/UIButton.cpp
        src/UI/UIInput.cpp
        src/UI/UIPanel.cpp
        src/UI/UIText.cpp
        src/UI/UIRadio.cpp
        src/UI/UIHudPanel.cpp
        src/UI/UIScoreboardPanel.cpp
        src/UI/UISlider.cpp
        src/UI/UIEventMenu.cpp
        src/Core.cpp
        src/CoreUpdate.cpp
        src/CoreUI.cpp
        include/Components/ComponentShared.hpp
        include/Components/ComponentInhabitant.hpp
        include/Components/ComponentTile.hpp
        include/Components/ComponentMusic.hpp
        include/UI/IUIComponent.hpp
        include/UI/AUIComponent.hpp
        include/UI/UIEvent.hpp
        include/UI/UIManager.hpp
        include/UI/UIButton.hpp
        include/UI/UIChatPanel.hpp
        include/UI/UIInput.hpp
        include/UI/UIPanel.hpp
        include/UI/UIText.hpp
        include/UI/UIRadio.hpp
        include/UI/UIHudPanel.hpp
        include/UI/UIScoreboardPanel.hpp
        include/UI/UISlider.hpp
        include/UI/UIEventMenu.hpp
        include/Systems/RenderSystem.hpp
        include/Graphics/AssetManager.hpp
        include/Core.hpp
        include/Commands/ACommand.hpp
        include/Commands/FactoryCommands.hpp
        include/Commands/CommandTimeUpdate.hpp
        include/Commands/CommandMapEvent.hpp
        include/Commands/CommandTileContent.hpp
        include/Commands/CommandTeamNames.hpp
        include/Commands/CommandPlayerPosition.hpp
        include/Commands/CommandPlayerLevel.hpp
        include/Commands/CommandPlayerInventory.hpp
        include/Commands/CommandMapSize.hpp
        include/Commands/CommandMapContent.hpp
        src/Commands/FactoryCommands.cpp
        include/ECS/Entity.hpp
        include/ECS/IComponentStorage.hpp
        include/ECS/World.hpp
)

# Create a core library to avoid redundant compilation
add_library(zappy_gui_core STATIC ${CORE_SOURCES})

target_include_directories(zappy_gui_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/angle/include
        ${raylib-cpp_SOURCE_DIR}/include
)

target_link_libraries(zappy_gui_core PUBLIC raylib)

target_compile_definitions(zappy_gui_core PRIVATE ZAPPY_LOG_LEVEL=${ZAPPY_LOG_LEVEL})

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE zappy_gui_core)

target_compile_definitions(${PROJECT_NAME} PRIVATE ZAPPY_LOG_LEVEL=${ZAPPY_LOG_LEVEL})

if (APPLE)
  target_link_libraries(zappy_gui_core PUBLIC
            "-framework IOKit"
            "-framework Cocoa"
            "-framework CoreFoundation"
            "-framework CoreVideo"
            "${CMAKE_CURRENT_SOURCE_DIR}/vendor/angle/libEGL.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/vendor/angle/libGLESv2.dylib"
    )

  # Copy dylibs to build folder and fix paths for runtime loading
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/vendor/angle/libEGL.dylib" $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/vendor/angle/libGLESv2.dylib" $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    COMMAND install_name_tool -change "./libEGL.dylib" "@executable_path/libEGL.dylib" $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND install_name_tool -change "./libGLESv2.dylib" "@executable_path/libGLESv2.dylib" $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Copying ANGLE libraries and fixing runtime paths..."
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
