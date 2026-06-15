cmake_minimum_required(VERSION 3.28)
project(zappy_server_project)

corrosion_import_crate(
        MANIFEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml"
        CRATE_TYPES bin
)

add_custom_target(zappy_server ALL)
add_dependencies(zappy_server cargo-build_zappy_server_rust)
