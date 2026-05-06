#!/usr/bin/env python3

import os
import platform
import subprocess
import shutil
import sys
import time
import itertools
import threading

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

class WinProgress:
    HIDDEN        = 0
    NORMAL        = 1
    ERROR         = 2
    INDETERMINATE = 3
    WARNING       = 4

    _enabled = 'WT_SESSION' in os.environ or os.getenv("TERM_PROGRAM") == "ghostty"

    @staticmethod
    def _write(mode, value=0):
        if not WinProgress._enabled:
            return
        sys.stdout.write(f'\x1b]9;4;{mode};{value}\x07')
        sys.stdout.flush()

    @staticmethod
    def start():
        WinProgress._write(WinProgress.INDETERMINATE)

    @staticmethod
    def set(pct: int):
        WinProgress._write(WinProgress.NORMAL, max(0, min(100, pct)))

    @staticmethod
    def error():
        WinProgress._write(WinProgress.ERROR, 100)

    @staticmethod
    def done():
        WinProgress._write(WinProgress.HIDDEN)

class Spinner:
    def __init__(self, message="Loading..."):
        self.spinner = itertools.cycle(['⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏'])
        self.busy = False
        self.delay = 0.1
        self.message = message
        self.thread = None
        self._lock = threading.Lock()

    def _spin(self):
        while self.busy:
            with self._lock:
                sys.stdout.write(f'\r{'\033[96m'}{next(self.spinner)}{'\033[0m'} {self.message}')
                sys.stdout.flush()
            time.sleep(self.delay)

    def __enter__(self):
        self.busy = True
        WinProgress.start()
        self.thread = threading.Thread(target=self._spin, daemon=True)
        self.thread.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.busy = False
        WinProgress.done()
        if self.thread:
            self.thread.join()
        sys.stdout.write('\r' + ' ' * (len(self.message) + 4) + '\r')
        sys.stdout.flush()

def run(cmd, silent=False):
    try:
        if silent:
            subprocess.check_call(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            subprocess.check_call(cmd, shell=True)
    except subprocess.CalledProcessError:
        WinProgress.error()
        print(f"\nCommand failed: {cmd}")
        sys.exit(1)

def print_progress(stage, task, end_spaces=40):
    line = f"{stage} {task}"
    print(f"\r{line}{' ' * end_spaces}", end="", flush=True)

def native_mode_enabled():
    return any(arg in ("-n", "--native") for arg in sys.argv[1:])

def try_phasor_toolchain():
    if native_mode_enabled():
        return False

    pulsar_exe = shutil.which("pulsar")  # for build.pul
    phasor_exe = shutil.which("phasor") or shutil.which("phasorvm")  # for running pmake
    phasorcompiler_exe = shutil.which("phasorcompiler")  # for compiling (required by pmake.conf)

    if pulsar_exe and phasor_exe and phasorcompiler_exe:
        with Spinner("Building pmake.phsb with phasor toolchain..."):
            run(f'"{pulsar_exe}" scripts/build.pul scripts/pmake.conf', silent=True)

        print()
        sys.exit(0)

    return False

commit_hash = subprocess.check_output(f"git -C {os.path.join(SCRIPT_DIR, 'thirdparty/pmake')} rev-parse --short HEAD", shell=True).decode().strip()
with Spinner("Syncing..."):
    run("git submodule update --init --recursive", silent=True)


try_phasor_toolchain()

WinProgress.start()

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
        WinProgress.error()
        print("\nNo Visual Studio installation with required C++ tools found")
        sys.exit(1)
    WinProgress.set(5)
    print_progress("[=---------]", "CFG vs-vcvars")
    run(f'"{os.path.join(vs_path, "Common7", "Tools", "VsDevCmd.bat")}" -arch={VSBUILD_ARCH} -host_arch={VSBUILD_HOST_ARCH} -no_logo', silent=True)
    print_progress("[==--------]", "CFG vs-vcvars done")
    WinProgress.set(10)
elif OS_NAME == "windows":
    print_progress("[==--------]", "VSCMD_VER detected, skipping VS setup")
    WinProgress.set(10)

print_progress("[==--------]", "CFG phasor-cmake")
WinProgress.set(20)
run(f'cmake -S "{SCRIPT_DIR}" -B "{TMP_DIR}" --preset {CMAKE_PRESET}', silent=True)
print_progress("[===-------]", "CFG phasor-cmake done")
WinProgress.set(30)

build_targets = ["phasor_native_runtime_static", "phasor_cxx_transpiler", "pmake"]
progress_marks = {"[===-------]": 40, "[=====-----]": 75, "[=======---]": 80}

for (mark, percent), target in zip(progress_marks.items(), build_targets):
    print_progress(mark, f"CXX {target}")
    WinProgress.set(percent)
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
WinProgress.set(95)

shutil.rmtree(TMP_DIR, ignore_errors=True)
print_progress("[==========]", "DONE")
WinProgress.done()
print()