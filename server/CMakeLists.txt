cmake_minimum_required(VERSION 3.28)
project(zappy_server_project)

corrosion_import_crate(
        MANIFEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml"
        CRATE_TYPES bin
)

add_custom_target(zappy_server ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/zappy_server_rust" "${CMAKE_BINARY_DIR}/zappy_server"
    DEPENDS cargo-build_zappy_server_rust
)

