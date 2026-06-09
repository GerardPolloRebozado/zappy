cmake_minimum_required(VERSION 3.28)
project(zappy_server)

corrosion_import_crate(
        MANIFEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml"
        CRATE_TYPES bin
)