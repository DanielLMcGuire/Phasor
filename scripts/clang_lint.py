#!/usr/bin/env python3

import argparse
import fnmatch
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from threading import Lock

DEFAULT_BUILD_DIR   = r".pmake/CMakeBuild"
DEFAULT_SOURCE_ROOT = "."
DEFAULT_LOG         = "clang-tidy.log"
DEFAULT_IGNORE      = ".clang-tidy-ignore"
DEFAULT_EXTENSIONS  = {".cpp", ".hpp", ".c", ".h"}
DEFAULT_WORKERS     = 8

def load_ignore_patterns(ignore_file: str) -> list[str]:
    path = Path(ignore_file)
    if not path.exists():
        return []
    patterns = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip().replace("\\", "/")
        if line and not line.startswith("#"):
            patterns.append(line)
    return patterns

def is_ignored(file_path: Path, patterns: list[str]) -> bool:
    path_str = str(file_path.resolve()).replace("\\", "/")
    name_str = file_path.name

    for pattern in patterns:
        if fnmatch.fnmatch(path_str, f"*{pattern}*"):
            return True
        if pattern in path_str:
            return True
        if fnmatch.fnmatch(name_str, pattern):
            return True
    return False

def collect_files(root: str, extensions: set[str], patterns: list[str]) -> list[Path]:
    root_path = Path(root).resolve()
    kept    = []
    skipped = []

    for p in sorted(root_path.rglob("*")):
        if p.suffix not in extensions:
            continue
        if is_ignored(p, patterns):
            skipped.append(p)
        else:
            kept.append(p)

    return kept

def run_tidy(file: Path, build_dir: str) -> tuple[Path, str]:
    result = subprocess.run(
        ["clang-tidy", str(file), "-p", build_dir],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return file, result.stdout

def main() -> None:
    parser = argparse.ArgumentParser(description="Parallel clang-tidy runner")
    parser.add_argument("--build-dir",   default=DEFAULT_BUILD_DIR,   help="compile_commands.json directory")
    parser.add_argument("--source-root", default=DEFAULT_SOURCE_ROOT, help="root directory to scan")
    parser.add_argument("--log",         default=DEFAULT_LOG,         help="output log file")
    parser.add_argument("--ignore-file", default=DEFAULT_IGNORE,      help="path patterns to skip (one per line)")
    parser.add_argument("--workers",     default=DEFAULT_WORKERS, type=int, help="parallel worker count")
    args = parser.parse_args()

    patterns = load_ignore_patterns(args.ignore_file)
    files    = collect_files(args.source_root, DEFAULT_EXTENSIONS, patterns)
    total    = len(files)

    if total == 0:
        print("nothing to do.")
        sys.exit(0)

    print(f"Processing {total} file(s) with {args.workers} worker(s)...\n")

    done       = 0
    lock       = Lock()
    all_output = []

    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(run_tidy, f, args.build_dir): f for f in files}
        for future in as_completed(futures):
            file, output = future.result()
            with lock:
                done += 1
                all_output.append(output)
                pct = int((done / total) * 100)
                print(f"\r[{done}/{total} {pct}%]", end="", flush=True)

    print()
    log_path = Path(args.log)
    log_path.write_text("\n".join(all_output), encoding="utf-8")
    print(f"\nDone — {total} file(s) processed. Output saved to '{log_path}'")

if __name__ == "__main__":
    main()
