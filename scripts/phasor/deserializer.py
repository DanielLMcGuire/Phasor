"""
phasor.deserializer
===================
Reads the binary  into an object.
"""

from __future__ import annotations

import struct
import zlib
from pathlib import Path

from .bytecode import Bytecode
from .instructions import Instruction
from .metadata import (
    HEADER_SIZE, MAGIC, SEC_CONSTANTS, SEC_FUNCTIONS,
    SEC_INSTRUCTIONS, SEC_VARIABLES, VERSION,
)
from .opcodes import OpCode
from .value import Value, ValueType


class BytecodeDeserializer:
    """Deserialises ``.phsb`` into :class:`~phasor.bytecode.Bytecode`."""

    def __init__(self) -> None:
        self._data: bytes = b""
        self._pos:  int   = 0

    def deserialize(self, data: bytes) -> Bytecode:
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
        data = Path(path).read_bytes()
        return self.deserialize(data)

    def _read_header(self) -> int:
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
            op4    = self._read_int32()
            op5    = self._read_int32()
            bytecode.instructions.append(Instruction(opcode, op1, op2, op3, op4, op5))

    def _read_value(self) -> Value:
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
        if self._pos + n > len(self._data):
            raise ValueError(
                f"Unexpected end of data at offset {self._pos} "
                f"(need {n} more bytes)"
            )

    def _read_uint8(self) -> int:
        self._require(1)
        v = self._data[self._pos]
        self._pos += 1
        return v

    def _read_uint16(self) -> int:
        self._require(2)
        (v,) = struct.unpack_from("<H", self._data, self._pos)
        self._pos += 2
        return v

    def _read_uint32(self) -> int:
        self._require(4)
        (v,) = struct.unpack_from("<I", self._data, self._pos)
        self._pos += 4
        return v

    def _read_int32(self) -> int:
        self._require(4)
        (v,) = struct.unpack_from("<i", self._data, self._pos)
        self._pos += 4
        return v

    def _read_int64(self) -> int:
        self._require(8)
        (v,) = struct.unpack_from("<q", self._data, self._pos)
        self._pos += 8
        return v

    def _read_double(self) -> float:
        self._require(8)
        (v,) = struct.unpack_from("<d", self._data, self._pos)
        self._pos += 8
        return v

    def _read_string(self) -> str:
        length = self._read_uint16()
        self._require(length)
        s = self._data[self._pos : self._pos + length].decode("utf-8")
        self._pos += length
        return s
