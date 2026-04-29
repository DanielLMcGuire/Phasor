#!/usr/bin/env python3
import argparse
import asyncio
import fnmatch
import sys
from pathlib import Path

DEFAULT_BUILD_DIR   = r".pmake/CMakeBuild"
DEFAULT_SOURCE_ROOT = "."
DEFAULT_LOG         = "clang-tidy.log"
DEFAULT_IGNORE      = ".clang-tidy-ignore"
DEFAULT_EXTENSIONS  = {".cpp", ".c", ".hpp", ".h"}
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
    kept = []
    for p in sorted(root_path.rglob("*")):
        if p.suffix not in extensions:
            continue
        if not is_ignored(p, patterns):
            kept.append(p)
    return kept


async def worker(
    queue: asyncio.Queue,
    results: list,
    clang_tidy: str,
    build_dir: str,
    state: dict,
    total: int,
) -> None:
    while True:
        try:
            file: Path = queue.get_nowait()
        except asyncio.QueueEmpty:
            return

        print(f"[{state['done']}/{total}] starting  {file.name}", flush=True)

        proc = await asyncio.create_subprocess_exec(
            clang_tidy, str(file), "-p", build_dir,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.STDOUT,
        )
        stdout, _ = await proc.communicate()
        output = stdout.decode(errors="replace")

        state["done"] += 1
        print(f"[{state['done']}/{total}] done      {file.name}", flush=True)

        results.append((str(file), output, proc.returncode))
        queue.task_done()


async def async_main(args: argparse.Namespace) -> int:
    patterns = load_ignore_patterns(args.ignore_file)
    files    = collect_files(args.source_root, DEFAULT_EXTENSIONS, patterns)
    total    = len(files)

    if total == 0:
        print("Nothing to do.")
        return 0

    print(f"Processing {total} file(s) with {args.workers} worker(s)...\n")

    queue = asyncio.Queue()
    for f in files:
        queue.put_nowait(f)

    results = []
    state   = {"done": 0}

    workers = [
        asyncio.create_task(worker(queue, results, args.clang_tidy, args.build_dir, state, total))
        for _ in range(min(args.workers, total))
    ]
    await asyncio.gather(*workers)

    had_errors = any(rc != 0 for _, _, rc in results)
    all_output = [f"=== {f} ===\n{o}" for f, o, _ in results]

    log_path = Path(args.log)
    log_path.write_text("\n\n".join(all_output), encoding="utf-8")
    print(f"\nDone — {total} file(s) processed. Log saved to '{log_path}'")

    if had_errors:
        print("Warnings/errors found. Check the log.")
        return 1
    return 0


def main() -> None:
    parser = argparse.ArgumentParser(description="Parallel clang-tidy runner")
    parser.add_argument("--build-dir",   default=DEFAULT_BUILD_DIR)
    parser.add_argument("--source-root", default=DEFAULT_SOURCE_ROOT)
    parser.add_argument("--log",         default=DEFAULT_LOG)
    parser.add_argument("--ignore-file", default=DEFAULT_IGNORE)
    parser.add_argument("--workers",     default=DEFAULT_WORKERS, type=int)
    parser.add_argument("--clang-tidy",  default="clang-tidy")
    args = parser.parse_args()

    sys.exit(asyncio.run(async_main(args)))


if __name__ == "__main__":
    main()