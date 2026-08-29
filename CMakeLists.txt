cmake_minimum_required(VERSION 3.20)
project(miqulauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure RPATH so the executable always finds libmiqutoolkit.so
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

find_package(PkgConfig REQUIRED)

# Support ~/.local pkgconfig directory
set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/.local/lib/pkgconfig:$ENV{HOME}/.local/share/pkgconfig:$ENV{PKG_CONFIG_PATH}")

pkg_check_modules(MIQUTOOLKIT REQUIRED IMPORTED_TARGET miqutoolkit)

add_executable(miqulauncher
    src/main.cpp
)

target_link_libraries(miqulauncher PRIVATE
    PkgConfig::MIQUTOOLKIT
)

target_compile_options(miqulauncher PRIVATE
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter
)

include(GNUInstallDirs)
install(TARGETS miqulauncher RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
