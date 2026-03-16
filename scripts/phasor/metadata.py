"""
phasor.metadata
===============
Binary format constants shared by the serializer and deserializer.
"""

def _ascii_to_u32_le(s: str) -> int:
    b = s.encode("ascii")
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)

MAGIC: int = _ascii_to_u32_le("PHSB")
VERSION: int = 0x03000000
HEADER_SIZE: int = 16       # bytes: MAGIC(4) + VERSION(4) + FLAGS(4) + CHECKSUM(4)

SEC_CONSTANTS:    int = 0x01
SEC_VARIABLES:    int = 0x02
SEC_INSTRUCTIONS: int = 0x03
SEC_FUNCTIONS:    int = 0x04