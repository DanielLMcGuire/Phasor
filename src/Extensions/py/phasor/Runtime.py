"""
phasor.Runtime
==============
cffi bindings for the PhasorRT shared library.

State is an opaque handle returned by :func:`new_state` and passed into
execution and evaluation functions.  This mirrors the C API directly
"""

from __future__ import annotations

import os
import sys
from pathlib import Path
from typing import Optional, Sequence, Union

from .Bytecode import Bytecode

from cffi import FFI

def _to_bytes(bytecode: "Union[bytes, bytearray, Bytecode]") -> bytes:
    """Accept raw bytes/bytearray *or* a Bytecode object; always return bytes."""
    if isinstance(bytecode, Bytecode):
        return bytecode.to_bytes()
    return bytes(bytecode)

_ffi = FFI()
_ffi.cdef(
    """
    int exec(void *state, const unsigned char *bytecode, size_t bytecodeSize,
             const char *moduleName, int argc, const char **argv);

    int evaluatePHS(void *state, const char *script, const char *moduleName,
                    const char *modulePath, bool verbose);

    int evaluatePUL(void *state, const char *script, const char *moduleName);

    bool compilePHS(const char *script, const char *moduleName,
                    const char *modulePath,
                    unsigned char *buffer, size_t bufferSize, size_t *outSize);

    bool compilePUL(const char *script, const char *moduleName,
                    unsigned char *buffer, size_t bufferSize, size_t *outSize);

    void *createState(void);

    void initStdLib(void *state);

    bool  freeState(void *state);

    bool  resetState(void *state, bool resetFunctions, bool resetVariables);
    """
)

_CANDIDATES: dict[str, list[str]] = {
    "linux":  ["libphasorrt.so", "libphasorrt.so.1"],
    "darwin": ["libphasorrt.dylib", "libphasorrt.1.dylib"],
    "win32":  ["phasorrt.dll"],
}

_LOAD_HINT: dict[str, str] = {
    "linux":  "Set LD_LIBRARY_PATH or run ldconfig after installing Phasor.",
    "darwin": "Set DYLD_LIBRARY_PATH or install Phasor.",
    "win32":  "Add phasorrt.dll to your system PATH",
}

_lib = None  # loaded lazily on first use


def _get_lib():
    global _lib
    if _lib is not None:
        return _lib

    key   = "linux" if sys.platform.startswith("linux") else sys.platform
    names = _CANDIDATES.get(key, _CANDIDATES["linux"])
    hint  = _LOAD_HINT.get(key, "Ensure the library is on your system library search path.")

    errors: list[str] = []
    for name in names:
        try:
            _lib = _ffi.dlopen(name)
            return _lib
        except OSError as exc:
            errors.append(f"  {name}: {exc}")

    raise OSError(
        f"Could not load PhasorRT shared library on {sys.platform}.\n"
        "Tried:\n" + "\n".join(errors) + f"\n{hint}"
    )

_DEFAULT_MODULE_NAME = "Python"

StateHandle = object  # cffi cdata void *


def _cwd_bytes() -> bytes:
    return os.getcwd().encode("utf-8")


def _ptr(state: Optional[StateHandle]):
    """Return the raw cffi pointer for *state*, or NULL if *state* is None."""
    return state if state is not None else _ffi.NULL


def _build_argv(args: Sequence[str]):
    """Convert a sequence of strings to ``(argc, argv_p)`` for cffi."""
    if not args:
        return 0, _ffi.NULL
    encoded = [_ffi.new("char[]", a.encode("utf-8")) for a in args]
    return len(encoded), _ffi.new("const char *[]", encoded)


def _two_pass_compile(compile_fn, *leading_args: bytes) -> Bytecode:
    """Size-probe then compile as required by the C API.

    Returns:
        A :class:`~phasor.Bytecode.Bytecode` object built from the compiled output.
    """
    out_size = _ffi.new("size_t *")

    if not compile_fn(*leading_args, _ffi.NULL, 0, out_size):
        raise RuntimeError("Compilation failed, check your source for errors.")

    size = out_size[0]
    buf  = _ffi.new(f"unsigned char[{size}]")

    if not compile_fn(*leading_args, buf, size, out_size):
        raise RuntimeError("Compilation failed during buffer write; this is likely a PhasorRT bug.")

    return Bytecode.from_bytes(bytes(_ffi.buffer(buf, size)))

def new_state() -> StateHandle:
    """Allocate a new Phasor VM state and return its opaque handle.

    Pass the handle to any execution or evaluation function via the
    ``state`` parameter to reuse the same VM across multiple calls,
    preserving globals and registered functions between them.

    Always pair with :func:`free_state` when the state is no longer needed.

    Returns:
        An opaque cffi handle representing the new VM state.

    Raises:
        RuntimeError: If ``createState()`` returns NULL.
        OSError: If the PhasorRT library cannot be loaded.

    Example::

        vm = new_state()
        free_state(vm)
    """
    lib = _get_lib()
    ptr = lib.createState()
    if ptr == _ffi.NULL:
        raise RuntimeError("createState() returned NULL; PhasorRT may be uninitialised.")
    return ptr

def init_stdlib(state: StateHandle) -> None:
    """Register standard library functions into a given state.

    Args: 
        state: The handle returned by :func:`new_state`.

    Raises:
        RuntimeError: If ``initStdLib()`` reports failure.
    """
    if not _get_lib().initStdLib(state):
        raise RuntimeError("initStdLib() failed.")

def free_state(state: StateHandle) -> None:
    """Release a VM state created by :func:`new_state`.

    The handle must not be used after this call.

    Args:
        state: The handle returned by :func:`new_state`.

    Raises:
        RuntimeError: If ``freeState()`` reports failure (double-free or corrupt pointer).
        OSError: If the PhasorRT library cannot be loaded.
    """
    if not _get_lib().freeState(state):
        raise RuntimeError("freeState() failed; the state may already have been freed.")


def reset_state(
    state: StateHandle,
    *,
    reset_functions: bool = False,
    reset_variables: bool = False,
) -> None:
    """Reset a VM state (stack, PC, and bytecode are always cleared).

    Args:
        state: The handle returned by :func:`new_state`.
        reset_functions: Also clear all registered functions when ``True``.
        reset_variables: Also clear all global variables when ``True``.

    Raises:
        RuntimeError: If ``resetState()`` reports failure.
        OSError: If the PhasorRT library cannot be loaded.

    Example::

        # Soft reset: keep globals and functions, just reset execution state.
        reset_state(vm)

        # Full wipe.
        reset_state(vm, reset_functions=True, reset_variables=True)
    """
    if not _get_lib().resetState(state, reset_functions, reset_variables):
        raise RuntimeError("resetState() failed; state may be corrupt.")

def compile_phs(
    script: str,
    *,
    module_name: str = _DEFAULT_MODULE_NAME,
    module_path: Optional[str] = None,
) -> bytes:
    """Compile a Phasor (``.phs``) source string to ``.phsb`` bytecode.

    Args:
        script: Phasor source code to compile.
        module_name: Name reported in error messages. Defaults to ``"Python"``.
        module_path: Directory for resolving compile-time imports.
            Defaults to the current working directory.

    Returns:
        A :class:`~phasor.Bytecode.Bytecode` object, ready to inspect, modify,
        pass to :func:`run`, or serialise with
        :meth:`~phasor.Bytecode.Bytecode.save` / :meth:`~phasor.Bytecode.Bytecode.to_bytes`.

    Raises:
        RuntimeError: If compilation fails.
        OSError: If the PhasorRT library cannot be loaded.
    """
    mod_path = module_path.encode("utf-8") if module_path else _cwd_bytes()
    return _two_pass_compile(
        _get_lib().compilePHS,
        script.encode("utf-8"),
        module_name.encode("utf-8"),
        mod_path,
    )


def compile_phs_file(
    path: str | Path,
    *,
    module_name: Optional[str] = None,
    module_path: Optional[str] = None,
) -> bytes:
    """Read a ``.phs`` file and compile it to ``.phsb`` bytecode.

    Args:
        path: Path to the ``.phs`` source file.
        module_name: Name reported in error messages.
            Defaults to the file's stem (e.g. ``"hello"`` for ``hello.phs``).
        module_path: Directory for resolving compile-time imports.
            Defaults to the parent directory of *path*.

    Returns:
        A :class:`~phasor.Bytecode.Bytecode` object.

    Raises:
        FileNotFoundError: If *path* does not exist.
        RuntimeError: If compilation fails.
        OSError: If the PhasorRT library cannot be loaded.
    """
    p = Path(path)
    if not p.is_file():
        raise FileNotFoundError(f"Source file not found: {p}")
    return compile_phs(
        p.read_text(encoding="utf-8"),
        module_name=module_name or p.stem,
        module_path=module_path or str(p.resolve().parent),
    )


def compile_pul(
    script: str,
    *,
    module_name: str = _DEFAULT_MODULE_NAME,
) -> bytes:
    """Compile a Pulsar (``.pul``) source string to ``.phsb`` bytecode.

    Args:
        script: Pulsar source code to compile.
        module_name: Name reported in error messages. Defaults to ``"Python"``.

    Returns:
        A :class:`~phasor.Bytecode.Bytecode` object.

    Raises:
        RuntimeError: If compilation fails.
        OSError: If the PhasorRT library cannot be loaded.
    """
    return _two_pass_compile(
        _get_lib().compilePUL,
        script.encode("utf-8"),
        module_name.encode("utf-8"),
    )


def compile_pul_file(
    path: str | Path,
    *,
    module_name: Optional[str] = None,
) -> bytes:
    """Read a ``.pul`` file and compile it to ``.phsb`` bytecode.

    Args:
        path: Path to the ``.pul`` source file.
        module_name: Name reported in error messages.
            Defaults to the file's stem.

    Returns:
        A :class:`~phasor.Bytecode.Bytecode` object.

    Raises:
        FileNotFoundError: If *path* does not exist.
        RuntimeError: If compilation fails.
        OSError: If the PhasorRT library cannot be loaded.
    """
    p = Path(path)
    if not p.is_file():
        raise FileNotFoundError(f"Source file not found: {p}")
    return compile_pul(
        p.read_text(encoding="utf-8"),
        module_name=module_name or p.stem,
    )

def run(
    bytecode: Union[bytes, bytearray, Bytecode],
    *,
    state: Optional[StateHandle] = None,
    module_name: str = _DEFAULT_MODULE_NAME,
    args: Sequence[str] = (),
) -> int:
    """Execute pre-compiled ``.phsb`` bytecode.

    Args:
        bytecode: Raw ``.phsb`` bytes **or** a :class:`~phasor.Bytecode.Bytecode`
            object (e.g. from :func:`compile_phs` or
            :meth:`~phasor.Bytecode.Bytecode.load`).
        state: An optional handle from :func:`new_state`. When ``None``,
            PhasorRT creates and manages a transient state internally.
        module_name: Name reported in error messages. Defaults to ``"Python"``.
        args: Command-line arguments forwarded to the script as ``argv``.

    Returns:
        The script's exit code (``-1`` may indicate an unhandled VM exception).

    Raises:
        OSError: If the PhasorRT library cannot be loaded.
    """
    raw        = _to_bytes(bytecode)
    data       = _ffi.from_buffer("unsigned char[]", raw)
    argc, argv = _build_argv(args)
    return _get_lib().exec(
        _ptr(state), data, len(raw),
        module_name.encode("utf-8"), argc, argv,
    )


def run_file(
    path: str | Path,
    *,
    state: Optional[StateHandle] = None,
    module_name: Optional[str] = None,
    args: Sequence[str] = (),
) -> int:
    """Load a ``.phsb`` file and execute it.

    Args:
        path: Path to the ``.phsb`` bytecode file.
        state: An optional handle from :func:`new_state`.
        module_name: Name reported in error messages.
            Defaults to the file's stem.
        args: Command-line arguments forwarded to the script.

    Returns:
        The script's exit code.

    Raises:
        FileNotFoundError: If *path* does not exist.
        OSError: If the PhasorRT library cannot be loaded.
    """
    p = Path(path)
    if not p.is_file():
        raise FileNotFoundError(f"Bytecode file not found: {p}")
    return run(
        p.read_bytes(),
        state=state,
        module_name=module_name or p.stem,
        args=args,
    )


def evaluate_phs(
    script: str,
    *,
    state: Optional[StateHandle] = None,
    module_name: str = _DEFAULT_MODULE_NAME,
    module_path: Optional[str] = None,
    verbose: bool = False,
) -> int:
    """Compile and execute a Phasor source string.

    Args:
        script: Phasor source code.
        state: An optional handle from :func:`new_state`. When ``None``,
            PhasorRT creates and manages a transient state internally.
        module_name: Name reported in error messages. Defaults to ``"Python"``.
        module_path: Directory for resolving compile-time imports.
            Defaults to the current working directory.
        verbose: Print the AST to stdout when ``True``.

    Returns:
        The script's exit code.

    Raises:
        OSError: If the PhasorRT library cannot be loaded.
    """
    mod_path = module_path.encode("utf-8") if module_path else _cwd_bytes()
    return _get_lib().evaluatePHS(
        _ptr(state),
        script.encode("utf-8"),
        module_name.encode("utf-8"),
        mod_path,
        verbose,
    )


def evaluate_phs_file(
    path: str | Path,
    *,
    state: Optional[StateHandle] = None,
    module_name: Optional[str] = None,
    verbose: bool = False,
) -> int:
    """Read and evaluate a ``.phs`` source file.

    The file's parent directory is automatically used for resolving
    compile-time imports.

    Args:
        path: Path to the ``.phs`` source file.
        state: An optional handle from :func:`new_state`.
        module_name: Name reported in error messages.
            Defaults to the file's stem.
        verbose: Print the AST to stdout when ``True``.

    Returns:
        The script's exit code.

    Raises:
        FileNotFoundError: If *path* does not exist.
        OSError: If the PhasorRT library cannot be loaded.
    """
    p = Path(path)
    if not p.is_file():
        raise FileNotFoundError(f"Source file not found: {p}")
    return evaluate_phs(
        p.read_text(encoding="utf-8"),
        state=state,
        module_name=module_name or p.stem,
        module_path=str(p.resolve().parent),
        verbose=verbose,
    )


def evaluate_pul(
    script: str,
    *,
    state: Optional[StateHandle] = None,
    module_name: str = _DEFAULT_MODULE_NAME,
) -> int:
    """Compile and execute a Pulsar source string.

    Args:
        script: Pulsar source code.
        state: An optional handle from :func:`new_state`. When ``None``,
            PhasorRT creates and manages a transient state internally.
        module_name: Name reported in error messages. Defaults to ``"Python"``.

    Returns:
        The script's exit code.

    Raises:
        OSError: If the PhasorRT library cannot be loaded.
    """
    return _get_lib().evaluatePUL(
        _ptr(state),
        script.encode("utf-8"),
        module_name.encode("utf-8"),
    )


def evaluate_pul_file(
    path: str | Path,
    *,
    state: Optional[StateHandle] = None,
    module_name: Optional[str] = None,
) -> int:
    """Read and evaluate a ``.pul`` source file.

    Args:
        path: Path to the ``.pul`` source file.
        state: An optional handle from :func:`new_state`.
        module_name: Name reported in error messages.
            Defaults to the file's stem.

    Returns:
        The script's exit code.

    Raises:
        FileNotFoundError: If *path* does not exist.
        OSError: If the PhasorRT library cannot be loaded.
    """
    p = Path(path)
    if not p.is_file():
        raise FileNotFoundError(f"Source file not found: {p}")
    return evaluate_pul(
        p.read_text(encoding="utf-8"),
        state=state,
        module_name=module_name or p.stem,
    )