#!/bin/bash
set -e

if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

cmake -S . -B build -G Ninja -DASSEMBLY=OFF
cmake --build build --target phasor_compiler
cmake --build build --target phasor_repl
cmake --build build --target phasor_runtime_exe
cmake --build build --target phasor_interpreter

convert_ico_to_png() {
    local ico_file=$1
    local png_file=$2
    
    if [ -f "$ico_file" ] && [ ! -f "$png_file" ]; then
        convert "$ico_file" -thumbnail 256x256 -alpha on -background none -flatten "$png_file"
    fi
}
convert_ico_to_png "assets/Compiler.ico" "phasorcompiler.png"
convert_ico_to_png "assets/Repl.ico" "phasorrepl.png"
convert_ico_to_png "assets/BinaryRuntime.ico" "phasorvm.png"
convert_ico_to_png "assets/ScriptingRuntime.ico" "phasorjit.png"

build_with_linuxdeploy() {
    local executable=$1
    local appname=$2
    local icon_file=$3
    
    local desktop_file=$(mktemp --suffix=.desktop)
    cat > "$desktop_file" <<EOF
[Desktop Entry]
Type=Application
Name=$appname
Exec=$executable
Icon=$executable
Categories=Development;
Terminal=true
EOF
    
    ./linuxdeploy-x86_64.AppImage \
        --appdir AppDir-temp \
        --executable="build/$executable" \
        --desktop-file="$desktop_file" \
        --icon-file="$icon_file" \
        --output appimage

    rm "$desktop_file"
	rm "$icon_file"
    rm -rf AppDir-temp
}
build_with_linuxdeploy "phasorcompiler" "Phasor Compiler" "phasorcompiler.png"
build_with_linuxdeploy "phasorrepl" "Phasor REPL" "phasorrepl.png"
build_with_linuxdeploy "phasorvm" "Phasor Runtime" "phasorvm.png"
build_with_linuxdeploy "phasorjit" "Phasor Interpreter" "phasorjit.png"