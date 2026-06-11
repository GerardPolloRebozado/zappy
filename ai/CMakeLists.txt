cmake_minimum_required(VERSION 3.10)
project(ZappyAI)

# Find Python 3
find_package(Python3 REQUIRED)

# "Compile" the AI: Copy the script and make it executable, then symlink to root
add_custom_target(zappy_ai ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/main.py" "${CMAKE_CURRENT_BINARY_DIR}/zappy_ai"
    COMMAND chmod +x "${CMAKE_CURRENT_BINARY_DIR}/zappy_ai"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_BINARY_DIR}/zappy_ai" "${CMAKE_CURRENT_SOURCE_DIR}/zappy_ai"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.py"
    COMMENT "Creating zappy_ai executable and symlink"
)

add_custom_target(train
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR} ${Python3_EXECUTABLE} training/training_env/train.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Starting Zappy AI Training (PPO)..."
    USES_TERMINAL
)

# Testing configuration
option(BUILD_TESTING "Enable tests" ON)

if(BUILD_TESTING)
    enable_testing()

    # Find all test files matching test_*.py in tests/
    file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.py")

    foreach(test_file ${TEST_FILES})
        get_filename_component(test_name ${test_file} NAME_WE)
        add_test(
            NAME ${test_name}
            COMMAND ${Python3_EXECUTABLE} -m unittest ${test_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()
endif()
