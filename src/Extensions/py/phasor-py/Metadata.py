"""
phasor-py.Metadata
================
Binary format constants shared by the Serializer and Deserializer.
"""

# Copyright 2026 Daniel McGuire
# Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
# Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http:#www.apache.org/licenses/LICENSE-2.0
# or https:#phasor.pages.dev/LICENSE.txt
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

def _ascii_to_u32_le(s: str) -> int:
    """Pack a 4-character ASCII string into a little-endian uint32 magic number."""
    b = s.encode("ascii")
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)

MAGIC: int = _ascii_to_u32_le("PHSB")
VERSION: int = 0x03000000
HEADER_SIZE: int = 16       # bytes: MAGIC(4) + VERSION(4) + FLAGS(4) + CHECKSUM(4)

SEC_CONSTANTS:    int = 0x01
SEC_VARIABLES:    int = 0x02
SEC_INSTRUCTIONS: int = 0x03
SEC_FUNCTIONS:    int = 0x04
SEC_STRUCTS:      int = 0x05
SEC_FUNC_TYPES:   int = 0x06
SEC_SCOPE_VARS:   int = 0x07