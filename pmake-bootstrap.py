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
                sys.stdout.write(f'\r{"\033[96m"}{next(self.spinner)}{"\033[0m"} {self.message}')
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

class ProgressBar:
    FILL      = '='
    EMPTY     = '-'
    MIN_BAR_W = 10
    LABEL_W   = 28

    def __init__(self):
        self._pct   = 0
        self._label = ""

    def _cols(self) -> int:
        return shutil.get_terminal_size(fallback=(80, 24)).columns

    def _render(self, pct: int, label: str) -> str:
        pct       = max(0, min(100, pct))
        label_str = label.ljust(self.LABEL_W)[:self.LABEL_W]
        pct_str   = f"{pct:3d}%"

        overhead  = self.LABEL_W + 3 + 2 + len(pct_str)
        bar_w     = max(self.MIN_BAR_W, self._cols() - overhead)

        filled = round(bar_w * pct / 100)
        bar    = self.FILL * filled + self.EMPTY * (bar_w - filled)

        return f"\r{label_str}  [{bar}] {pct_str}"

    def update(self, pct: int, label: str = "") -> None:
        self._pct   = pct
        self._label = label
        WinProgress.set(pct)
        sys.stdout.write(self._render(pct, label))
        sys.stdout.flush()

    def done(self, label: str = "DONE") -> None:
        self.update(100, label)
        WinProgress.done()
        sys.stdout.write('\n')
        sys.stdout.flush()

    def error(self, label: str = "FAILED") -> None:
        WinProgress.error()
        sys.stdout.write(self._render(self._pct, label) + '\n')
        sys.stdout.flush()

progress = ProgressBar()

def run(cmd, silent=False):
    try:
        if silent:
            subprocess.check_call(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            subprocess.check_call(cmd, shell=True)
    except subprocess.CalledProcessError:
        progress.error(f"FAILED: {cmd}")
        sys.exit(1)


def native_mode_enabled():
    return any(arg in ("-n", "--native") for arg in sys.argv[1:])


def force_mode_enabled():
    return any(arg in ("-f", "--force") for arg in sys.argv[1:])

def detect_platform():
    system = platform.system().lower()
    if system == "darwin":
        os_name = "macos"
    elif system == "windows":
        os_name = "windows"
    else:
        os_name = "linux"

    machine = platform.machine().lower()
    if machine in ("x86_64", "amd64"):
        arch = "64"
    elif machine in ("i386", "i686", "x86"):
        arch = "32"
    elif machine in ("arm64", "aarch64"):
        arch = "arm64"
    else:
        print(f"Unsupported architecture: {machine}")
        sys.exit(1)

    if arch == "32":
        cmake_preset = f"{os_name}-32-rel"
        vsbuild_arch = "x86" if os_name == "windows" else None
        vsbuild_host_arch = "x86" if os_name == "windows" else None
    elif arch == "64":
        cmake_preset = f"{os_name}-64-rel"
        vsbuild_arch = "x86" if os_name == "windows" else None
        vsbuild_host_arch = "x64" if os_name == "windows" else None
    else:
        cmake_preset = f"{os_name}-arm64-rel"
        vsbuild_arch = "arm64" if os_name == "windows" else None
        vsbuild_host_arch = "x64" if os_name == "windows" else None

    return os_name, arch, cmake_preset, vsbuild_arch, vsbuild_host_arch


def check_existing_build():
    if force_mode_enabled():
        return

    pmake_names = ["pmake", "pmake.phsb"]
    if platform.system().lower() == "windows":
        pmake_names.append("pmake.exe")

    existing = next(
        (os.path.join(SCRIPT_DIR, name) for name in pmake_names
         if os.path.exists(os.path.join(SCRIPT_DIR, name))),
        None,
    )
    if existing:
        print(f"Found existing build: {os.path.basename(existing)}, skipping build.")
        sys.exit(0)


def sync_submodules():
    with Spinner("Syncing..."):
        run("git submodule update --init --recursive", silent=True)


def try_phasor_toolchain():
    if native_mode_enabled():
        return False

    pulsar_exe        = shutil.which("pulsar")
    phasor_exe        = shutil.which("phasor") or shutil.which("phasorvm")
    phasorcompiler_exe = shutil.which("phasorcompiler")

    if pulsar_exe and phasor_exe and phasorcompiler_exe:
        with Spinner("Building pmake.phsb with phasor toolchain..."):
            run(f'"{pulsar_exe}" scripts/build.pul scripts/pmake.conf', silent=True)
        print()
        sys.exit(0)

    return False


def setup_vs_environment(vsbuild_arch, vsbuild_host_arch):
    if "VSCMD_VER" in os.environ:
        progress.update(10, "CFG vs-vcvars skipped")
        return

    vswhere_path = os.path.join(
        os.environ.get("ProgramFiles(x86)", ""),
        "Microsoft Visual Studio", "Installer", "vswhere.exe",
    )
    if not os.path.exists(vswhere_path):
        progress.error("CFG vs-vcvars — vswhere.exe not found")
        sys.exit(1)

    vs_path = subprocess.check_output(
        f'"{vswhere_path}" -latest -products * '
        f'-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath',
        shell=True,
    ).decode().strip()

    if not vs_path:
        progress.error("CFG vs-vcvars — no VS install found")
        sys.exit(1)

    progress.update(5, "CFG vs-vcvars")
    vcvars = os.path.join(vs_path, "Common7", "Tools", "VsDevCmd.bat")
    run(f'"{vcvars}" -arch={vsbuild_arch} -host_arch={vsbuild_host_arch} -no_logo', silent=True)
    progress.update(10, "CFG vs-vcvars done")


def cmake_configure(cmake_preset):
    progress.update(20, "CFG phasor-cmake")
    run(f'cmake -S "{SCRIPT_DIR}" -B "{TMP_DIR}" --preset {cmake_preset}', silent=True)
    progress.update(30, "CFG phasor-cmake done")


def build_and_install(os_name):
    build_targets = [
        ("phasor_native_runtime_static", 40, 55),
        ("phasor_cxx_transpiler",        55, 75),
        ("pmake",                         75, 85),
    ]

    for target, pct_start, pct_end in build_targets:
        progress.update(pct_start, f"CXX {target}")
        run(f'ninja -C "{TMP_DIR}" {target}', silent=True)
        progress.update(pct_end, f"CXX {target} done")

    if os_name == "windows":
        src = os.path.join(TMP_DIR, "thirdparty", "pmake", "Executable", "pmake", "pmake.exe")
        dst = os.path.join(SCRIPT_DIR, "pmake.exe")
    else:
        src = os.path.join(TMP_DIR, "thirdparty", "pmake", "Executable", "pmake", "pmake")
        dst = os.path.join(SCRIPT_DIR, "pmake")

    progress.update(90, f"INST {os.path.basename(dst)}")
    shutil.copyfile(src, dst)

    shutil.rmtree(TMP_DIR, ignore_errors=True)
    progress.done("DONE")

def main():
    os_name, arch, cmake_preset, vsbuild_arch, vsbuild_host_arch = detect_platform()

    check_existing_build()

    subprocess.check_output(
        f"git -C {os.path.join(SCRIPT_DIR, 'thirdparty/pmake')} rev-parse --short HEAD",
        shell=True,
    ).decode().strip()

    sync_submodules()
    try_phasor_toolchain()

    progress.update(0, "Starting build")

    if os_name == "windows":
        setup_vs_environment(vsbuild_arch, vsbuild_host_arch)

    cmake_configure(cmake_preset)
    build_and_install(os_name)


if __name__ == "__main__":
    main()