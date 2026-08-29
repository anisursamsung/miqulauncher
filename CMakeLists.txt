cmake_minimum_required(VERSION 3.20)
project(biwaymenu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure RPATH so the executable always finds libbiwaytoolkit.so
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

find_package(PkgConfig REQUIRED)

# Support ~/.local pkgconfig directory
set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/.local/lib/pkgconfig:$ENV{HOME}/.local/share/pkgconfig:$ENV{PKG_CONFIG_PATH}")

pkg_check_modules(BIWAYTOOLKIT REQUIRED IMPORTED_TARGET biwaytoolkit)

add_executable(biwaymenu
    src/main.cpp
)

target_link_libraries(biwaymenu PRIVATE
    PkgConfig::BIWAYTOOLKIT
)

target_compile_options(biwaymenu PRIVATE
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter
)

include(GNUInstallDirs)
install(TARGETS biwaymenu RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
