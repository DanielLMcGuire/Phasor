#!/usr/bin/env bash

(return 0 2>/dev/null) # check if we are sourced

if [ "$?" -ne 0 ]; then
    echo "Warning: This script should be sourced!" >&2
    echo "Usage: source ${BASH_SOURCE[0]:-$0}" >&2
fi

CURRENT_DIR="$(pwd)"

OS_NAME="$(uname -s)"

case "$OS_NAME" in
    Darwin)
        export PHASOR_INCLUDE_PATH="${CURRENT_DIR}/out/Library/Application Support/org.Phasor.Phasor/"
        export PHASOR_DLL_PATH="${CURRENT_DIR}/out/usr/local/bin/libphasorrt.dylib"
        ;;
    *)
        export PHASOR_INCLUDE_PATH="${CURRENT_DIR}/out/opt/phasor"
        export PHASOR_DLL_PATH="${CURRENT_DIR}/out/usr/bin/libphasorrt.so"
        ;;
esac

echo "PHASOR_INCLUDE_PATH set to: ${PHASOR_INCLUDE_PATH}"
echo "PHASOR_DLL_PATH set to: ${PHASOR_DLL_PATH}"