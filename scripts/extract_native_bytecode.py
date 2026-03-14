#!/usr/bin/env python3

import sys
import struct
import argparse
from pathlib import Path
from typing import Optional
import lief

def arch_info(binary) -> tuple[int, str]:
    try:
        fmt = binary.format
        if fmt == lief.Binary.FORMATS.ELF:
            cls  = binary.header.identity_class
            data = binary.header.identity_data
            bits = 64 if cls == lief.ELF.ELF_CLASS.CLASS64 else 32
            end  = '<' if data == lief.ELF.ELF_DATA.ELFDATA2LSB else '>'
            return (8 if bits == 64 else 4), end
        if fmt == lief.Binary.FORMATS.PE:
            magic = binary.optional_header.magic
            bits  = 64 if magic == lief.PE.PE_TYPE.PE32_PLUS else 32
            return (8 if bits == 64 else 4), '<'
        if fmt == lief.Binary.FORMATS.MACHO:
            cpu  = binary.header.cpu_type
            bits = 64 if cpu in (lief.MachO.CPU_TYPES.ARM64,
                                  lief.MachO.CPU_TYPES.x86_64) else 32
            return (8 if bits == 64 else 4), '<'
    except Exception:
        pass
    return 8, '<'


def find_phasor_section(binary) -> Optional[object]:
    candidates = {'.phsb', 'phsb'}
    try:
        for sec in binary.sections:
            if sec.name.rstrip('\x00') in candidates:
                return sec
    except Exception:
        pass
    try:
        for seg in binary.segments:
            for sec in seg.sections:
                if sec.name.rstrip('\x00') in candidates:
                    return sec
    except Exception:
        pass
    return None

def find_all(data: bytes, pattern: bytes) -> list[int]:
    offsets, start = [], 0
    while True:
        pos = data.find(pattern, start)
        if pos == -1:
            break
        offsets.append(pos)
        start = pos + 1
    return offsets


def find_size(sec_data: bytes, sz_width: int, endian: str) -> Optional[int]:
    fmt      = endian + ('Q' if sz_width == 8 else 'I')
    L        = len(sec_data)
    non_zero = [i for i in range(L) if sec_data[i] != 0]

    for padding in range(L - sz_width):
        N = L - sz_width - padding
        if N <= 0:
            break

        for sz_off in find_all(sec_data, struct.pack(fmt, N)):
            sz_end     = sz_off + sz_width
            outside_nz = [i for i in non_zero if i < sz_off or i >= sz_end]

            if not outside_nz:
                return N

            span_start = outside_nz[0]
            span_end   = outside_nz[-1] + 1

            if span_end - span_start > N:
                continue

            for bc_start in range(max(0, span_end - N), min(span_start, L - N) + 1):
                bc_end = bc_start + N
                if bc_start < sz_end and bc_end > sz_off:
                    continue
                return N

    return None


def main():
    ap = argparse.ArgumentParser(description="Extract phasor bytecode from a compiled binary.")
    ap.add_argument("binary", help="Path to the compiled binary")
    ap.add_argument("-o", "--output", default="bytecode.phasor", help="Output file")
    args = ap.parse_args()

    binary_path = Path(args.binary)
    if not binary_path.is_file():
        print(f"Error: file not found: {binary_path}")
        sys.exit(1)

    binary = lief.parse(str(binary_path))
    if binary is None:
        print("Error: could not parse binary.")
        sys.exit(1)

    section = find_phasor_section(binary)
    if section is None:
        print("Error: no '.phsb' section found.")
        sys.exit(1)

    sz_width, endian = arch_info(binary)

    sec_data = bytes(section.content)

    N = find_size(sec_data, sz_width, endian)
    if N is None:
        print("Error: could not locate size_t in .phasor section.")
        sys.exit(1)

    phsb_off = sec_data.find(b"\x50\x48\x53\x42")
    if phsb_off == -1:
        print("Error: PHSB magic not found in .phasor section.")
        sys.exit(1)

    output_path = Path(args.output)
    output_path.write_bytes(sec_data[phsb_off : phsb_off + N])
    print(f"Extracted {N} bytes -> {output_path}")

if __name__ == "__main__":
    main()