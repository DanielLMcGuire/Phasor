import pefile
import sys
import struct
from pathlib import Path

if len(sys.argv) < 2:
    print("Usage: extract.py <input.exe> [output.bin]")
    sys.exit(1)

input_path = Path(sys.argv[1])
output_path = Path(sys.argv[2]) if len(sys.argv) > 2 else input_path.with_suffix(".phsb")

pe = pefile.PE(str(input_path))

for section in pe.sections:
    if section.Name.rstrip(b'\x00') == b'.phasor':
        data = section.get_data()
        length = struct.unpack_from('<Q', data, 0)[0]
        bytecode = data[8:8 + length]
        
        with open(output_path, "wb") as f:
            f.write(bytecode)

        break
else:
    print("Error: Not a valid Phasor Runtime Executable!")
    sys.exit(1)