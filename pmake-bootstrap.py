#!/usr/bin/env python

import os
import platform
import subprocess
import shutil
import sys
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TMP_DIR = os.path.join(SCRIPT_DIR, ".tmp")

PLATFORM_SYSTEM = platform.system().lower()
if PLATFORM_SYSTEM == "darwin":
    OS_NAME = "macos"
elif PLATFORM_SYSTEM == "windows":
    OS_NAME = "windows"
else:
    OS_NAME = "linux"

ARCH_MACHINE = platform.machine().lower()
if ARCH_MACHINE in ("x86_64", "amd64"):
    ARCH = "64"
elif ARCH_MACHINE in ("i386", "i686", "x86"):
    ARCH = "32"
elif ARCH_MACHINE in ("arm64", "aarch64"):
    ARCH = "arm64"
else:
    print(f"Unsupported architecture: {ARCH_MACHINE}")
    sys.exit(1)

if ARCH == "32":
    CMAKE_PRESET = f"{OS_NAME}-32-rel"
    VSBUILD_ARCH = "x86" if OS_NAME == "windows" else None
    VSBUILD_HOST_ARCH = "x86" if OS_NAME == "windows" else None
elif ARCH == "64":
    CMAKE_PRESET = f"{OS_NAME}-64-rel"
    VSBUILD_ARCH = "x86" if OS_NAME == "windows" else None
    VSBUILD_HOST_ARCH = "x64" if OS_NAME == "windows" else None
elif ARCH == "arm64":
    CMAKE_PRESET = f"{OS_NAME}-arm64-rel"
    VSBUILD_ARCH = "arm64" if OS_NAME == "windows" else None
    VSBUILD_HOST_ARCH = "x64" if OS_NAME == "windows" else None

def run(cmd, silent=False):
    try:
        if silent:
            subprocess.check_call(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            subprocess.check_call(cmd, shell=True)
    except subprocess.CalledProcessError:
        print(f"\nCommand failed: {cmd}")
        sys.exit(1)

def print_progress(stage, task, end_spaces=40):
    line = f"{stage} {task}"
    print(f"\r{line}{' ' * end_spaces}", end="", flush=True)

commit_hash = subprocess.check_output(f"git -C {os.path.join(SCRIPT_DIR, 'thirdparty/pmake')} rev-parse --short HEAD", shell=True).decode().strip()
print_progress("[----------]", f"SYNC pmake {commit_hash}")
run("git submodule update --init", silent=True)
print_progress("[----------]", "SYNC done")

if OS_NAME == "windows" and "VSCMD_VER" not in os.environ:
    vswhere_path = os.path.join(os.environ.get("ProgramFiles(x86)"), "Microsoft Visual Studio", "Installer", "vswhere.exe")
    if not os.path.exists(vswhere_path):
        print("\nvswhere.exe not found")
        sys.exit(1)
    vs_path = subprocess.check_output(
        f'"{vswhere_path}" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath',
        shell=True,
    ).decode().strip()
    if not vs_path:
        print("\nNo Visual Studio installation with required C++ tools found")
        sys.exit(1)
    print_progress("[=---------]", "CFG vs-vcvars")
    run(f'"{os.path.join(vs_path, "Common7", "Tools", "VsDevCmd.bat")}" -arch={VSBUILD_ARCH} -host_arch={VSBUILD_HOST_ARCH} -no_logo', silent=True)
    print_progress("[==--------]", "CFG vs-vcvars done")
elif OS_NAME == "windows":
    print_progress("[=---------]", "VSCMD_VER detected, skipping VS setup")

print_progress("[==--------]", "CFG phasor-cmake")
run(f'cmake -S "{SCRIPT_DIR}" -B "{TMP_DIR}" --preset {CMAKE_PRESET}', silent=True)
print_progress("[===-------]", "CFG phasor-cmake done")

build_targets = ["phasor_native_runtime_static", "phasor_cxx_transpiler", "pmake"]
progress_marks = ["[===-------]", "[=====-----]", "[=======---]"]

for mark, target in zip(progress_marks, build_targets):
    print_progress(mark, f"CXX {target}")
    run(f'ninja -C "{TMP_DIR}" {target}', silent=True)
    print_progress(mark, f"{target} done")

if OS_NAME == "windows":
    src_pmake = os.path.join(TMP_DIR, "thirdparty", "pmake", "Executable", "pmake", "pmake.exe")
    dst_pmake = os.path.join(SCRIPT_DIR, "pmake.exe")
else:
    src_pmake = os.path.join(TMP_DIR, "thirdparty", "pmake", "Executable", "pmake", "pmake")
    dst_pmake = os.path.join(SCRIPT_DIR, "pmake")

shutil.copyfile(src_pmake, dst_pmake)
print_progress("[=========-]", f"INST {os.path.basename(dst_pmake)} done")

shutil.rmtree(TMP_DIR, ignore_errors=True)
print_progress("[==========]", "DONE")
print()