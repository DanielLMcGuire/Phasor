# Serializer.py
"""
phasor.Serializer
=================
Serialises a :class:`~phasor.Bytecode.Bytecode` object to the binary ``.phsb`` format.
"""

from __future__ import annotations

import struct
import zlib
from pathlib import Path
from typing import Dict, List

from .Bytecode import Bytecode, StructInfo
from .Instruction import Instruction
from .Metadata import (
    HEADER_SIZE, MAGIC, SEC_CONSTANTS, SEC_FUNCTIONS,
    SEC_FUNC_TYPES, SEC_INSTRUCTIONS, SEC_STRUCTS, SEC_VARIABLES, VERSION,
)
from .Value import Value, ValueType


class BytecodeSerializer:
    """Converts a :class:`~phasor.Bytecode.Bytecode` object into its binary ``.phsb`` representation."""

    def __init__(self) -> None:
        """Initialise the serializer with an empty write buffer."""
        self._buf: bytearray = bytearray()

    def serialize(self, bytecode: Bytecode) -> bytes:
        """Serialise *bytecode* to the ``.phsb`` wire format.

        Writes a 16-byte header (magic, version, flags, CRC-32 checksum) followed
        by the constants, variables, functions, function-types, structs, and
        instructions sections in order.

        Args:
            bytecode: The :class:`~phasor.Bytecode.Bytecode` object to serialise.

        Returns:
            The complete ``.phsb`` binary as a :class:`bytes` object.
        """
        self._buf = bytearray()

        self._buf.extend(bytes(HEADER_SIZE))
        data_start = len(self._buf)

        self._write_constant_pool(bytecode.constants)
        self._write_variable_mapping(bytecode.variables, bytecode.next_var_index)
        self._write_scope_vars(bytecode.scope_var_lists)
        self._write_function_entries(bytecode.function_entries)
        self._write_function_types(bytecode)
        self._write_struct_section(bytecode.structs)
        self._write_instructions(bytecode.instructions)

        checksum = zlib.crc32(self._buf[data_start:]) & 0xFFFFFFFF

        header = struct.pack("<IIII", MAGIC, VERSION, 0, checksum)
        self._buf[:HEADER_SIZE] = header

        return bytes(self._buf)

    def save_to_file(self, bytecode: Bytecode, path: Path) -> None:
        """Serialise *bytecode* and write the result to *path*.

        Parent directories are created automatically if they do not exist.

        Args:
            bytecode: The :class:`~phasor.Bytecode.Bytecode` object to serialise.
            path: Destination file path; typically ends with ``.phsb``.
        """
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(self.serialize(bytecode))

    def _write_constant_pool(self, constants: List[Value]) -> None:
        """Write the :data:`~phasor.Metadata.SEC_CONSTANTS` section: count followed by each :class:`~phasor.Value.Value`."""
        self._write_uint8(SEC_CONSTANTS)
        self._write_uint32(len(constants))
        for value in constants:
            self._write_value(value)

    def _write_variable_mapping(
        self, variables: Dict[str, int], next_var_index: int
    ) -> None:
        """Write the :data:`~phasor.Metadata.SEC_VARIABLES` section: count, :attr:`~phasor.Bytecode.Bytecode.next_var_index`, then each name→slot pair."""
        self._write_uint8(SEC_VARIABLES)
        self._write_uint32(len(variables))
        self._write_int32(next_var_index)
        for name, index in variables.items():
            self._write_string(name)
            self._write_int32(index)

    def _write_scope_vars(self, scope_var_lists: list[list[tuple[int, str]]]) -> None:
        self._write_uint8(SEC_SCOPE_VARS)
        self._write_uint32(len(scope_var_lists))
        for scope in scope_var_lists:
            self._write_uint32(len(scope))
            for var_index, var_name in scope:
                self._write_int32(var_index)
                self._write_string(var_name)

    def _write_function_entries(self, function_entries: Dict[str, int]) -> None:
        """Write the :data:`~phasor.Metadata.SEC_FUNCTIONS` section: count then each name→instruction-index entry point."""
        self._write_uint8(SEC_FUNCTIONS)
        self._write_uint32(len(function_entries))
        for name, address in function_entries.items():
            self._write_string(name)
            self._write_int32(address)

    def _write_function_types(self, bytecode: Bytecode) -> None:
        """Write the :data:`~phasor.Metadata.SEC_FUNC_TYPES` section (0x06).

        Binary layout per entry::

            string  name
            string  returnTypeName   ("any" when absent)
            uint32  paramCount
            string  paramTypeName[0..paramCount-1]

        Only functions present in
        :attr:`~phasor.Bytecode.Bytecode.function_param_type_names` are emitted;
        untyped functions are omitted from this section.
        """
        self._write_uint8(SEC_FUNC_TYPES)
        self._write_uint32(len(bytecode.function_param_type_names))
        for name, param_types in bytecode.function_param_type_names.items():
            self._write_string(name)
            self._write_string(bytecode.function_return_type_names.get(name, "any"))
            self._write_uint32(len(param_types))
            for type_name in param_types:
                self._write_string(type_name)

    def _write_struct_section(self, structs: List[StructInfo]) -> None:
        """Write the :data:`~phasor.Metadata.SEC_STRUCTS` section (0x05).

        Binary layout per entry::

            string  name
            int32   firstConstIndex
            int32   fieldCount
            string  fieldName[0..fieldCount-1]
        """
        self._write_uint8(SEC_STRUCTS)
        self._write_uint32(len(structs))
        for info in structs:
            self._write_string(info.name)
            self._write_int32(info.first_const_index)
            self._write_int32(info.field_count)
            for field_name in info.field_names:
                self._write_string(field_name)

    def _write_instructions(self, instructions: List[Instruction]) -> None:
        """Write the :data:`~phasor.Metadata.SEC_INSTRUCTIONS` section: count then each :class:`~phasor.Instruction.Instruction` as ``uint8`` opcode + three ``int32`` operands."""
        self._write_uint8(SEC_INSTRUCTIONS)
        self._write_uint32(len(instructions))
        for instr in instructions:
            self._write_uint8(int(instr.op))
            self._write_int32(instr.operand1)
            self._write_int32(instr.operand2)
            self._write_int32(instr.operand3)

    def _write_value(self, value: Value) -> None:
        """Write a :class:`~phasor.Value.Value` as a ``uint8`` type tag followed by its payload.

        Tags and layouts::

            0  Null
            1  Bool   : uint8(0|1)
            2  Int    : int64
            3  Float  : float64
            4  String : uint16(len) + bytes
            5  Struct : string(structName) uint32(fieldCount) [string(name) value]...
            6  Array  : uint32(elementCount) [value]...

        Raises:
            NotImplementedError: If :attr:`value.type <phasor.Value.Value.type>` is not a
                recognised :class:`~phasor.Value.ValueType`.
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
        elif t == ValueType.Struct:
            s = value.as_struct()
            self._write_uint8(5)
            self._write_string(s.struct_name)
            self._write_uint32(len(s.fields))
            for field_name, field_val in s.fields.items():
                self._write_string(field_name)
                self._write_value(field_val)
        elif t == ValueType.Array:
            elems = value.as_array()
            self._write_uint8(6)
            self._write_uint32(len(elems))
            for elem in elems:
                self._write_value(elem)
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