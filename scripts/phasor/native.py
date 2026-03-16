"""
phasor.native
=============
Extract bytecode from a ELF / PE / MachO binary.
"""

from __future__ import annotations

import struct
from pathlib import Path
from typing import Optional, Tuple

_MAGIC_BYTES = b"\x50\x48\x53\x42"

def _arch_info(binary) -> Tuple[int, str]:
    try:
        import lief
        fmt = binary.format
        if fmt == lief.Binary.FORMATS.ELF:
            cls  = binary.header.identity_class
            data = binary.header.identity_data
            bits = 64 if cls == lief.ELF.ELF_CLASS.CLASS64 else 32
            end  = "<" if data == lief.ELF.ELF_DATA.ELFDATA2LSB else ">"
            return (8 if bits == 64 else 4), end
        if fmt == lief.Binary.FORMATS.PE:
            magic = binary.optional_header.magic
            bits  = 64 if magic == lief.PE.PE_TYPE.PE32_PLUS else 32
            return (8 if bits == 64 else 4), "<"
        if fmt == lief.Binary.FORMATS.MACHO:
            cpu  = binary.header.cpu_type
            bits = 64 if cpu in (
                lief.MachO.CPU_TYPES.ARM64,
                lief.MachO.CPU_TYPES.x86_64,
            ) else 32
            return (8 if bits == 64 else 4), "<"
    except Exception:
        pass
    return 8, "<"


def _find_phasor_section(binary) -> Optional[object]:
    candidates = {".phsb", "phsb"}
    try:
        for sec in binary.sections:
            if sec.name.rstrip("\x00") in candidates:
                return sec
    except Exception:
        pass
    try:
        for seg in binary.segments:
            for sec in seg.sections:
                if sec.name.rstrip("\x00") in candidates:
                    return sec
    except Exception:
        pass
    return None


def _find_all(data: bytes, pattern: bytes) -> list[int]:
    offsets, start = [], 0
    while True:
        pos = data.find(pattern, start)
        if pos == -1:
            break
        offsets.append(pos)
        start = pos + 1
    return offsets


def _find_payload_size(sec_data: bytes, sz_width: int, endian: str) -> Optional[int]:
    fmt      = endian + ("Q" if sz_width == 8 else "I")
    L        = len(sec_data)
    non_zero = [i for i in range(L) if sec_data[i] != 0]

    for padding in range(L - sz_width):
        N = L - sz_width - padding
        if N <= 0:
            break

        for sz_off in _find_all(sec_data, struct.pack(fmt, N)):
            sz_end     = sz_off + sz_width
            outside_nz = [i for i in non_zero if i < sz_off or i >= sz_end]

            if not outside_nz:
                return N

            span_start = outside_nz[0]
            span_end   = outside_nz[-1] + 1

            if span_end - span_start > N:
                continue

            for bc_start in range(
                max(0, span_end - N), min(span_start, L - N) + 1
            ):
                bc_end = bc_start + N
                if bc_start < sz_end and bc_end > sz_off:
                    continue
                return N

    return None

def extract_phsb_bytes(path: Path) -> bytes:
    try:
        import lief
    except ImportError as exc:
        raise ImportError(
            "The 'lief' package is required for native binary extraction.\n"
            "Install it with: pip install lief"
        ) from exc

    path = Path(path)
    if not path.is_file():
        raise FileNotFoundError(f"Binary not found: {path}")

    binary = lief.parse(str(path))
    if binary is None:
        raise RuntimeError(f"Could not parse binary: {path}")

    section = _find_phasor_section(binary)
    if section is None:
        raise RuntimeError(f"No '.phsb' section found in {path}")

    sz_width, endian = _arch_info(binary)
    sec_data = bytes(section.content)

    magic_off = sec_data.find(_MAGIC_BYTES)
    if magic_off == -1:
        raise RuntimeError("PHSB magic bytes not found in '.phsb' section")

    N = _find_payload_size(sec_data, sz_width, endian)
    if N is None:
        raise RuntimeError(
            "Could not determine bytecode payload size from '.phsb' section"
        )

    return sec_data[magic_off : magic_off + N]
