# Deserializer.py
"""
phasor.Deserializer
===================
Deserialises the binary ``.phsb`` format into a :class:`~phasor.Bytecode.Bytecode` object.
"""

from __future__ import annotations

import struct
import zlib
from pathlib import Path

from .Bytecode import Bytecode, StructInfo
from .Instruction import Instruction
from .Metadata import (
    HEADER_SIZE, MAGIC, SEC_CONSTANTS, SEC_FUNCTIONS,
    SEC_FUNC_TYPES, SEC_INSTRUCTIONS, SEC_STRUCTS, SEC_VARIABLES, VERSION,
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
        self._read_scope_vars(bytecode)
        self._read_function_entries(bytecode)
        self._read_function_types(bytecode)
        self._read_struct_section(bytecode)
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

    def _read_scope_vars(self, bytecode: Bytecode) -> None:
        section_id = self._read_uint8()
        if section_id != SEC_SCOPE_VARS:
            raise ValueError(
                f"Expected scope vars section (0x{SEC_SCOPE_VARS:02x}), "
                f"got 0x{section_id:02x}"
            )
        count = self._read_uint32()
        for _ in range(count):
            list_size = self._read_uint32()
            scope: list[tuple[int, str]] = []
            for _ in range(list_size):
                idx  = self._read_int32()
                name = self._read_string()
                scope.append((idx, name))
            bytecode.scope_var_lists.append(scope)

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

    def _read_function_types(self, bytecode: Bytecode) -> None:
        """Read the :data:`~phasor.Metadata.SEC_FUNC_TYPES` section (0x06) and populate
        :attr:`~phasor.Bytecode.Bytecode.function_param_type_names`,
        :attr:`~phasor.Bytecode.Bytecode.function_return_type_names`, and
        :attr:`~phasor.Bytecode.Bytecode.function_param_counts`.
        """
        section_id = self._read_uint8()
        if section_id != SEC_FUNC_TYPES:
            raise ValueError(
                f"Expected function-types section (0x{SEC_FUNC_TYPES:02x}), "
                f"got 0x{section_id:02x}"
            )
        count = self._read_uint32()
        for _ in range(count):
            name        = self._read_string()
            return_type = self._read_string()
            param_count = self._read_uint32()
            param_types = [self._read_string() for _ in range(param_count)]
            bytecode.function_return_type_names[name] = return_type
            bytecode.function_param_type_names[name]  = param_types
            bytecode.function_param_counts[name]       = param_count

    def _read_struct_section(self, bytecode: Bytecode) -> None:
        """Read the :data:`~phasor.Metadata.SEC_STRUCTS` section (0x05) and populate
        :attr:`~phasor.Bytecode.Bytecode.structs` and
        :attr:`~phasor.Bytecode.Bytecode.struct_entries`.
        """
        section_id = self._read_uint8()
        if section_id != SEC_STRUCTS:
            raise ValueError(
                f"Expected structs section (0x{SEC_STRUCTS:02x}), "
                f"got 0x{section_id:02x}"
            )
        count = self._read_uint32()
        for i in range(count):
            name              = self._read_string()
            first_const_index = self._read_int32()
            field_count       = self._read_int32()
            field_names       = [self._read_string() for _ in range(field_count)]
            info = StructInfo(
                name=name,
                first_const_index=first_const_index,
                field_count=field_count,
                field_names=field_names,
            )
            bytecode.structs.append(info)
            bytecode.struct_entries[name] = i

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
        """Read a type-tagged value and return the corresponding :class:`~phasor.Value.Value`.

        Tags::

            0  Null
            1  Bool   : uint8(0|1)
            2  Int    : int64
            3  Float  : float64
            4  String : uint16(len) + bytes
            5  Struct : string(structName) uint32(fieldCount) [string(name) value]...
            6  Array  : uint32(elementCount) [value]...
        """
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
        if tag == 5:
            struct_name = self._read_string()
            field_count = self._read_uint32()
            fields: dict = {}
            for _ in range(field_count):
                field_name = self._read_string()
                fields[field_name] = self._read_value()
            return Value.from_struct(struct_name, fields)
        if tag == 6:
            elem_count = self._read_uint32()
            elems = [self._read_value() for _ in range(elem_count)]
            return Value.from_array(elems)
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