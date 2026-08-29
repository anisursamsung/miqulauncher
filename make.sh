#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
BUILD_TYPE="${BUILD_TYPE:-Release}"

export PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig:$HOME/.local/share/pkgconfig:${PKG_CONFIG_PATH:-}"

if [ "${EUID}" -eq 0 ]; then
    PREFIX="/usr"
else
    PREFIX="${PREFIX:-$HOME/.local}"
fi

echo "==> Configuring biwaymenu ($BUILD_TYPE)..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_INSTALL_PREFIX="$PREFIX" "$@"

echo "==> Building biwaymenu..."
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo "==> Installing biwaymenu to $PREFIX/bin..."
cmake --install "$BUILD_DIR"

echo "==> biwaymenu build and installation finished successfully!"
