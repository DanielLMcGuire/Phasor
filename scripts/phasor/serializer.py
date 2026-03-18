"""
phasor.serializer
=================
Serialises a :class:`~phasor.bytecode.Bytecode` object to the binary ``.phsb`` format.
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
    """Converts a :class:`~phasor.bytecode.Bytecode` object into its binary ``.phsb`` representation."""

    def __init__(self) -> None:
        """Initialise the serializer with an empty write buffer."""
        self._buf: bytearray = bytearray()


    def serialize(self, bytecode: Bytecode) -> bytes:
        """Serialise *bytecode* to the ``.phsb`` wire format.

        Writes a 16-byte header (magic, version, flags, CRC-32 checksum) followed
        by the constants, variables, functions, and instructions sections in order.

        Args:
            bytecode: The :class:`~phasor.bytecode.Bytecode` object to serialise.

        Returns:
            The complete ``.phsb`` binary as a :class:`bytes` object.
        """
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
        """Serialise *bytecode* and write the result to *path*.

        Parent directories are created automatically if they do not exist.

        Args:
            bytecode: The :class:`~phasor.bytecode.Bytecode` object to serialise.
            path: Destination file path; typically ends with ``.phsb``.
        """
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(self.serialize(bytecode))

    def _write_constant_pool(self, constants: List[Value]) -> None:
        """Write the :data:`~phasor.metadata.SEC_CONSTANTS` section: count followed by each :class:`~phasor.value.Value`."""
        self._write_uint8(SEC_CONSTANTS)
        self._write_uint32(len(constants))
        for value in constants:
            self._write_value(value)

    def _write_variable_mapping(
        self, variables: Dict[str, int], next_var_index: int
    ) -> None:
        """Write the :data:`~phasor.metadata.SEC_VARIABLES` section: count, :attr:`~phasor.bytecode.Bytecode.next_var_index`, then each name→slot pair."""
        self._write_uint8(SEC_VARIABLES)
        self._write_uint32(len(variables))
        self._write_int32(next_var_index)
        for name, index in variables.items():
            self._write_string(name)
            self._write_int32(index)

    def _write_function_entries(self, function_entries: Dict[str, int]) -> None:
        """Write the :data:`~phasor.metadata.SEC_FUNCTIONS` section: count then each name→instruction-index entry point."""
        self._write_uint8(SEC_FUNCTIONS)
        self._write_uint32(len(function_entries))
        for name, address in function_entries.items():
            self._write_string(name)
            self._write_int32(address)

    def _write_instructions(self, instructions: List[Instruction]) -> None:
        """Write the :data:`~phasor.metadata.SEC_INSTRUCTIONS` section: count then each :class:`~phasor.instructions.Instruction` as ``uint8`` opcode + five ``int32`` operands."""
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
        """Write a :class:`~phasor.value.Value` as a ``uint8`` type tag followed by its payload.

        Raises:
            NotImplementedError: If :attr:`value.type <phasor.value.Value.type>` is not one of
                Null, Bool, Int, Float, or String.
        """
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
        """Append a single unsigned byte to the buffer."""
        self._buf.append(v & 0xFF)

    def _write_uint16(self, v: int) -> None:
        """Append a little-endian unsigned 16-bit integer to the buffer."""
        self._buf.extend(struct.pack("<H", v & 0xFFFF))

    def _write_uint32(self, v: int) -> None:
        """Append a little-endian unsigned 32-bit integer to the buffer."""
        self._buf.extend(struct.pack("<I", v & 0xFFFFFFFF))

    def _write_int32(self, v: int) -> None:
        """Append a little-endian signed 32-bit integer to the buffer."""
        self._buf.extend(struct.pack("<i", v))

    def _write_int64(self, v: int) -> None:
        """Append a little-endian signed 64-bit integer to the buffer."""
        self._buf.extend(struct.pack("<q", v))

    def _write_double(self, v: float) -> None:
        """Append a little-endian IEEE 754 double to the buffer."""
        self._buf.extend(struct.pack("<d", v))

    def _write_string(self, s: str) -> None:
        """Append a length-prefixed UTF-8 string (uint16 length + bytes) to the buffer."""
        encoded = s.encode("utf-8")
        self._write_uint16(len(encoded))
        self._buf.extend(encoded)
