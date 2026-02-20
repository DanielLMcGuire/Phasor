#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TMP_DIR="$SCRIPT_DIR/.tmp"
BUILD_TYPE="rel"

if [ ! -d "$SCRIPT_DIR" ]; then
    echo "Cannot determine script directory"
    exit 1
fi

OS_NAME=$(uname -s)
case "$OS_NAME" in
    Darwin)
        OS="macos"
        ;;
    Linux)
        OS="linux"
        ;;
    *)
        OS="linux"
        ;;
esac

ARCH_NAME=$(uname -m)
case "$ARCH_NAME" in
    x86_64|amd64)
        ARCH=64
        ;;
    i386|i686)
        ARCH=32
        ;;
    arm64|aarch64)
        ARCH=arm64
        ;;
    *)
        echo "Unsupported architecture: $ARCH_NAME"
        exit 1
        ;;
esac

CMAKE_PRESET="${OS}-${ARCH}-${BUILD_TYPE}"

echo "SYNC"
git submodule update --init --recursive

echo "CFG phasor-cmake"
cmake -S "$SCRIPT_DIR" -B "$TMP_DIR" --preset "$CMAKE_PRESET"

echo "CXX phasor_native_runtime_static"
ninja -C "$TMP_DIR" phasor_native_runtime_static

echo "CXX phasor_cxx_transpiler"
ninja -C "$TMP_DIR" phasor_cxx_transpiler

echo "PHS pmake.phs"
ninja -C "$TMP_DIR" pmake

echo "INST pmake"
cp "$TMP_DIR/thirdparty/pmake/Executable/pmake/pmake" "$SCRIPT_DIR/pmake"

echo "CLEAN"
rm -rf "$TMP_DIR"

echo "DONE"
echo "Setting up build..."
"$SCRIPT_DIR/pmake" "${OS}-${ARCH}-${BUILD_TYPE}" -f

echo "Showing options..."
"$SCRIPT_DIR/pmake" -h