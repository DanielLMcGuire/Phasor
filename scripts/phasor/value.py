"""
phasor.values
=============
Runtime value types stored in the constant pool.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum
from typing import Union


class ValueType(IntEnum):
    Null   = 0
    Bool   = 1
    Int    = 2
    Float  = 3
    String = 4

_Payload = Union[None, bool, int, float, str]


@dataclass
class Value:
    """
    A typed constant-pool entry.

    Use the class-method constructors rather than setting ``type`` /
    ``_data`` directly::

        Value.null()
        Value.from_bool(True)
        Value.from_int(42)
        Value.from_float(3.14)
        Value.from_string("hello")
    """

    type:  ValueType
    _data: _Payload = None

    @classmethod
    def null(cls) -> "Value":
        return cls(ValueType.Null, None)

    @classmethod
    def from_bool(cls, v: bool) -> "Value":
        return cls(ValueType.Bool, bool(v))

    @classmethod
    def from_int(cls, v: int) -> "Value":
        return cls(ValueType.Int, int(v))

    @classmethod
    def from_float(cls, v: float) -> "Value":
        return cls(ValueType.Float, float(v))

    @classmethod
    def from_string(cls, v: str) -> "Value":
        return cls(ValueType.String, str(v))

    def as_bool(self) -> bool:
        if self.type != ValueType.Bool:
            raise TypeError(f"Value is {self.type.name}, not Bool")
        return bool(self._data)

    def as_int(self) -> int:
        if self.type != ValueType.Int:
            raise TypeError(f"Value is {self.type.name}, not Int")
        return int(self._data)  # type: ignore[arg-type]

    def as_float(self) -> float:
        if self.type != ValueType.Float:
            raise TypeError(f"Value is {self.type.name}, not Float")
        return float(self._data)  # type: ignore[arg-type]

    def as_string(self) -> str:
        if self.type != ValueType.String:
            raise TypeError(f"Value is {self.type.name}, not String")
        return str(self._data)

    def __repr__(self) -> str:
        if self.type == ValueType.Null:
            return "Value(null)"
        return f"Value({self.type.name}, {self._data!r})"

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Value):
            return NotImplemented
        return self.type == other.type and self._data == other._data
