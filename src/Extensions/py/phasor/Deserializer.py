"""
phasor.Deserializer
===================
Deserialises the binary ``.phsb`` format into a :class:`~phasor.Bytecode.Bytecode` object.
"""

from __future__ import annotations

import struct
import zlib
from pathlib import Path

from .Bytecode import Bytecode
from .Instruction import Instruction
from .Metadata import (
    HEADER_SIZE, MAGIC, SEC_CONSTANTS, SEC_FUNCTIONS,
    SEC_INSTRUCTIONS, SEC_VARIABLES, VERSION,
)
from .OpCode import OpCode
from .Value import Value, ValueType


class BytecodeDeserializer:
    """Deserialises ``.phsb`` into :class:`~phasor.Bytecode.Bytecode`."""

    def __init__(self) -> None:
        """Initialise the deserializer with an empty data buffer and zero read position."""
        self._data: bytes = b""
        self._pos:  int   = 0

    def deserialize(self, data: bytes) -> Bytecode:
        """Parse a raw ``.phsb`` byte buffer into a :class:`~phasor.Bytecode.Bytecode` object.

        Args:
            data: Raw bytes of a ``.phsb`` file.

        Returns:
            A fully populated :class:`~phasor.Bytecode.Bytecode` instance.

        Raises:
            ValueError: If the magic number, version, or CRC-32 checksum is invalid,
                or if any section tag is unexpected.
        """
        self._data = data
        self._pos  = 0
        bytecode = Bytecode()
        checksum = self._read_header()

        data_start = self._pos
        actual_crc = zlib.crc32(self._data[data_start:]) & 0xFFFFFFFF
        if actual_crc != checksum:
            raise ValueError(
                f"Bytecode checksum mismatch: "
                f"expected {checksum:#010x}, got {actual_crc:#010x}"
            )

        self._read_constant_pool(bytecode)
        self._read_variable_mapping(bytecode)
        self._read_function_entries(bytecode)
        self._read_instructions(bytecode)

        return bytecode

    def load_from_file(self, path: Path) -> Bytecode:
        """Read a ``.phsb`` file from disk and deserialise it.

        Args:
            path: Path to the ``.phsb`` file to load.

        Returns:
            A fully populated :class:`~phasor.Bytecode.Bytecode` instance.
        """
        data = Path(path).read_bytes()
        return self.deserialize(data)

    def _read_header(self) -> int:
        """Read and validate the 16-byte file header.

        Returns:
            The CRC-32 checksum stored in the header, to be verified against
            the actual data after reading.

        Raises:
            ValueError: If the magic number does not equal :data:`~phasor.Metadata.MAGIC`
                or the version does not equal :data:`~phasor.Metadata.VERSION`.
        """
        magic = self._read_uint32()
        if magic != MAGIC:
            raise ValueError(
                f"Invalid magic number: expected {MAGIC:#010x}, got {magic:#010x}"
            )

        version = self._read_uint32()
        if version != VERSION:
            raise ValueError(
                f"Unsupported version: {version:#010x} (expected {VERSION:#010x})"
            )

        _flags = self._read_uint32()
        checksum = self._read_uint32()
        return checksum

    def _read_constant_pool(self, bytecode: Bytecode) -> None:
        """Read the :data:`~phasor.Metadata.SEC_CONSTANTS` section and append entries to :attr:`bytecode.constants <phasor.Bytecode.Bytecode.constants>`."""
        section_id = self._read_uint8()
        if section_id != SEC_CONSTANTS:
            raise ValueError(
                f"Expected constants section (0x{SEC_CONSTANTS:02x}), "
                f"got 0x{section_id:02x}"
            )
        count = self._read_uint32()
        for _ in range(count):
            bytecode.constants.append(self._read_value())

    def _read_variable_mapping(self, bytecode: Bytecode) -> None:
        """Read the :data:`~phasor.Metadata.SEC_VARIABLES` section and populate :attr:`bytecode.variables <phasor.Bytecode.Bytecode.variables>` and :attr:`~phasor.Bytecode.Bytecode.next_var_index`."""
        section_id = self._read_uint8()
        if section_id != SEC_VARIABLES:
            raise ValueError(
                f"Expected variables section (0x{SEC_VARIABLES:02x}), "
                f"got 0x{section_id:02x}"
            )
        count = self._read_uint32()
        bytecode.next_var_index = self._read_int32()
        for _ in range(count):
            name  = self._read_string()
            index = self._read_int32()
            bytecode.variables[name] = index

    def _read_function_entries(self, bytecode: Bytecode) -> None:
        """Read the :data:`~phasor.Metadata.SEC_FUNCTIONS` section and populate :attr:`bytecode.function_entries <phasor.Bytecode.Bytecode.function_entries>`."""
        section_id = self._read_uint8()
        if section_id != SEC_FUNCTIONS:
            raise ValueError(
                f"Expected functions section (0x{SEC_FUNCTIONS:02x}), "
                f"got 0x{section_id:02x}"
            )
        count = self._read_uint32()
        for _ in range(count):
            name    = self._read_string()
            address = self._read_int32()
            bytecode.function_entries[name] = address

    def _read_instructions(self, bytecode: Bytecode) -> None:
        """Read the :data:`~phasor.Metadata.SEC_INSTRUCTIONS` section and populate :attr:`bytecode.instructions <phasor.Bytecode.Bytecode.instructions>`."""
        section_id = self._read_uint8()
        if section_id != SEC_INSTRUCTIONS:
            raise ValueError(
                f"Expected instructions section (0x{SEC_INSTRUCTIONS:02x}), "
                f"got 0x{section_id:02x}"
            )
        count = self._read_uint32()
        for _ in range(count):
            opcode = OpCode(self._read_uint8())
            op1    = self._read_int32()
            op2    = self._read_int32()
            op3    = self._read_int32()
            bytecode.instructions.append(Instruction(opcode, op1, op2, op3))

    def _read_value(self) -> Value:
        """Read a type-tagged value and return the corresponding :class:`~phasor.Value.Value`."""
        tag = self._read_uint8()
        if tag == 0:
            return Value.null()
        if tag == 1:
            return Value.from_bool(self._read_uint8() != 0)
        if tag == 2:
            return Value.from_int(self._read_int64())
        if tag == 3:
            return Value.from_float(self._read_double())
        if tag == 4:
            return Value.from_string(self._read_string())
        raise ValueError(f"Unknown value type tag: {tag}")

    def _require(self, n: int) -> None:
        """Raise ``ValueError`` if fewer than *n* bytes remain in the buffer."""
        if self._pos + n > len(self._data):
            raise ValueError(
                f"Unexpected end of data at offset {self._pos} "
                f"(need {n} more bytes)"
            )

    def _read_uint8(self) -> int:
        """Read and return the next unsigned byte from the buffer."""
        self._require(1)
        v = self._data[self._pos]
        self._pos += 1
        return v

    def _read_uint16(self) -> int:
        """Read and return the next little-endian unsigned 16-bit integer from the buffer."""
        self._require(2)
        (v,) = struct.unpack_from("<H", self._data, self._pos)
        self._pos += 2
        return v

    def _read_uint32(self) -> int:
        """Read and return the next little-endian unsigned 32-bit integer from the buffer."""
        self._require(4)
        (v,) = struct.unpack_from("<I", self._data, self._pos)
        self._pos += 4
        return v

    def _read_int32(self) -> int:
        """Read and return the next little-endian signed 32-bit integer from the buffer."""
        self._require(4)
        (v,) = struct.unpack_from("<i", self._data, self._pos)
        self._pos += 4
        return v

    def _read_int64(self) -> int:
        """Read and return the next little-endian signed 64-bit integer from the buffer."""
        self._require(8)
        (v,) = struct.unpack_from("<q", self._data, self._pos)
        self._pos += 8
        return v

    def _read_double(self) -> float:
        """Read and return the next little-endian IEEE 754 double from the buffer."""
        self._require(8)
        (v,) = struct.unpack_from("<d", self._data, self._pos)
        self._pos += 8
        return v

    def _read_string(self) -> str:
        """Read a length-prefixed UTF-8 string (uint16 length + bytes) and return it."""
        length = self._read_uint16()
        self._require(length)
        s = self._data[self._pos : self._pos + length].decode("utf-8")
        self._pos += length
        return s
