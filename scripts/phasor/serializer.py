"""
phasor.serializer
=================
writes Bytecode to a binary
"""

from __future__ import annotations

import struct
import zlib
from pathlib import Path
from typing import Dict, List

from .bytecode import Bytecode
from .instructions import Instruction
from .metadata import (
    HEADER_SIZE, MAGIC, SEC_CONSTANTS, SEC_FUNCTIONS,
    SEC_INSTRUCTIONS, SEC_VARIABLES, VERSION,
)
from .value import Value, ValueType


class BytecodeSerializer:

    def __init__(self) -> None:
        self._buf: bytearray = bytearray()


    def serialize(self, bytecode: Bytecode) -> bytes:
        self._buf = bytearray()

        self._buf.extend(bytes(HEADER_SIZE))
        data_start = len(self._buf)

        self._write_constant_pool(bytecode.constants)
        self._write_variable_mapping(bytecode.variables, bytecode.next_var_index)
        self._write_function_entries(bytecode.function_entries)
        self._write_instructions(bytecode.instructions)

        checksum = zlib.crc32(self._buf[data_start:]) & 0xFFFFFFFF

        header = struct.pack("<IIII", MAGIC, VERSION, 0, checksum)
        self._buf[:HEADER_SIZE] = header

        return bytes(self._buf)

    def save_to_file(self, bytecode: Bytecode, path: Path) -> None:
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(self.serialize(bytecode))

    def _write_constant_pool(self, constants: List[Value]) -> None:
        self._write_uint8(SEC_CONSTANTS)
        self._write_uint32(len(constants))
        for value in constants:
            self._write_value(value)

    def _write_variable_mapping(
        self, variables: Dict[str, int], next_var_index: int
    ) -> None:
        self._write_uint8(SEC_VARIABLES)
        self._write_uint32(len(variables))
        self._write_int32(next_var_index)
        for name, index in variables.items():
            self._write_string(name)
            self._write_int32(index)

    def _write_function_entries(self, function_entries: Dict[str, int]) -> None:
        self._write_uint8(SEC_FUNCTIONS)
        self._write_uint32(len(function_entries))
        for name, address in function_entries.items():
            self._write_string(name)
            self._write_int32(address)

    def _write_instructions(self, instructions: List[Instruction]) -> None:
        self._write_uint8(SEC_INSTRUCTIONS)
        self._write_uint32(len(instructions))
        for instr in instructions:
            self._write_uint8(int(instr.op))
            self._write_int32(instr.operand1)
            self._write_int32(instr.operand2)
            self._write_int32(instr.operand3)
            self._write_int32(instr.operand4)
            self._write_int32(instr.operand5)

    def _write_value(self, value: Value) -> None:
        t = value.type
        if t == ValueType.Null:
            self._write_uint8(0)
        elif t == ValueType.Bool:
            self._write_uint8(1)
            self._write_uint8(1 if value.as_bool() else 0)
        elif t == ValueType.Int:
            self._write_uint8(2)
            self._write_int64(value.as_int())
        elif t == ValueType.Float:
            self._write_uint8(3)
            self._write_double(value.as_float())
        elif t == ValueType.String:
            self._write_uint8(4)
            self._write_string(value.as_string())
        else:
            raise NotImplementedError(f"Serialization not implemented for {t!r}")

    def _write_uint8(self, v: int) -> None:
        self._buf.append(v & 0xFF)

    def _write_uint16(self, v: int) -> None:
        self._buf.extend(struct.pack("<H", v & 0xFFFF))

    def _write_uint32(self, v: int) -> None:
        self._buf.extend(struct.pack("<I", v & 0xFFFFFFFF))

    def _write_int32(self, v: int) -> None:
        self._buf.extend(struct.pack("<i", v))

    def _write_int64(self, v: int) -> None:
        self._buf.extend(struct.pack("<q", v))

    def _write_double(self, v: float) -> None:
        self._buf.extend(struct.pack("<d", v))

    def _write_string(self, s: str) -> None:
        encoded = s.encode("utf-8")
        self._write_uint16(len(encoded))
        self._buf.extend(encoded)
