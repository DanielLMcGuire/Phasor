#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TMP_DIR="$SCRIPT_DIR/.tmp"
cmake -S "$SCRIPT_DIR" -B "$TMP_DIR" -DCMAKE_BUILD_TYPE=Release -G Ninja 2>&1 1>/dev/null
echo "Building..."
ninja -C "$TMP_DIR" pmake 2>&1 1>nul
echo  "Copying files..."
cp "$TMP_DIR/src/Executable/utils/pmake/pmake.exe" "$SCRIPT_DIR/pmake.exe" 2>&1 1>/dev/null
echo "Cleaning up..."
rm -rf "$TMP_DIR" 2>&1 1>/dev/null