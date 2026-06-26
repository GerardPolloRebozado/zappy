{
  pkgs,
  lib,
  config,
  inputs,
  ...
}:

{
  # https://devenv.sh/basics/
  env.GREET = "devenv";

  # Set up environment variables that CMake and Rust might need to locate libraries
  env.CMAKE_GENERATOR = "Ninja"; # Optional: Uses Ninja instead of Make for faster builds

  # Set LD_LIBRARY_PATH so GUI applications (like Rust winit) can find Wayland/X11 libraries at runtime
  env.LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
    pkgs.wayland
    pkgs.libxkbcommon
    pkgs.libGL
    pkgs.libdecor
    pkgs.libX11
    pkgs.libxcursor
    pkgs.libxi
    pkgs.libxrandr
    pkgs.zlib
  ];

  # https://devenv.sh/packages/
  packages = [
    pkgs.git
    pkgs.cmake
    pkgs.ninja
    pkgs.valgrind
    pkgs.clang
    pkgs.clang-analyzer
    pkgs.doxygen
    pkgs.pkg-config
    pkgs.wayland
    pkgs.wayland-protocols
    pkgs.libxkbcommon
    pkgs.libGL
    pkgs.libdecor
    pkgs.libX11
    pkgs.libxcursor
    pkgs.libxi
    pkgs.libxrandr
    pkgs.libxinerama
    pkgs.pre-commit
    pkgs.criterion
    pkgs.ruff
    pkgs.clang-tools
    pkgs.zlib
  ];

  # https://devenv.sh/languages/
  languages = {
    rust = {
      enable = true;
      # Enables the rust-analyzer component for IDE support
      components = [
        "rustc"
        "cargo"
        "clippy"
        "rustfmt"
        "rust-analyzer"
      ];
    };

    python = {
      enable = true;
      venv = {
        enable = true;
      };
    };
    cplusplus.enable = true;
  };

  # https://devenv.sh/scripts/
  scripts.hello.exec = ''
    export NIX_CFLAGS_COMPILE="-fno-omit-frame-pointer $NIX_CFLAGS_COMPILE"
    echo "hello from $GREET (with Rust, CMake, and Valgrind ready!)"
  '';

  # https://devenv.sh/basics/
  enterShell = ''
    hello
    git --version
    cargo --version
    cmake --version
    valgrind --version
  '';

  # https://devenv.sh/tests/
  enterTest = ''
    echo "Running tests"
    cargo test
  '';
}
