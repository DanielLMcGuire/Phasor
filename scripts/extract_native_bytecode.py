#!/usr/bin/env python

import sys
import argparse
from pathlib import Path
from phasor import Bytecode

def main() -> None:
    ap = argparse.ArgumentParser(description="Extract phasor bytecode from a compiled binary.")
    ap.add_argument("binary", help="Path to the compiled binary")
    ap.add_argument("-o", "--output", default="out.phsb", help="Output file")
    args = ap.parse_args()
    output_path = Path(args.output)

    binary_path = Path(args.binary)
    if not binary_path.is_file():
        print(f"Error: file not found: {binary_path}")
        sys.exit(1)

    try:
        bc = Bytecode.from_native_binary(binary_path)
    except (ImportError, FileNotFoundError, RuntimeError) as exc:
        print(f"Error: {exc}")
        sys.exit(1)

    
    bc.save(output_path)
    print(f"Extracted to {output_path}")

if __name__ == "__main__":
    main()
