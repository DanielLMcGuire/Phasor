"""
phasor.Value
=============
Runtime value types stored in the constant pool.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum
from typing import Dict, List, Tuple, Union


class ValueType(IntEnum):
    """Type of a constant-pool :class:`Value`."""
    Null   = 0
    Bool   = 1
    Int    = 2
    Float  = 3
    String = 4
    Struct = 5
    Array  = 6

_Payload = Union[None, bool, int, float, str, Tuple[str, Dict[str, "Value"]], List["Value"]]


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
        Value.from_struct("MyStruct", {"x": Value.from_int(1)})
        Value.from_array([Value.from_int(1), Value.from_int(2)])
    """

    type:  ValueType
    _data: _Payload = None

    @classmethod
    def null(cls) -> "Value":
        """Return a new Null-typed value."""
        return cls(ValueType.Null, None)

    @classmethod
    def from_bool(cls, v: bool) -> "Value":
        """Return a new Bool-typed value wrapping *v*."""
        return cls(ValueType.Bool, bool(v))

    @classmethod
    def from_int(cls, v: int) -> "Value":
        """Return a new Int-typed value wrapping *v*."""
        return cls(ValueType.Int, int(v))

    @classmethod
    def from_float(cls, v: float) -> "Value":
        """Return a new Float-typed value wrapping *v*."""
        return cls(ValueType.Float, float(v))

    @classmethod
    def from_string(cls, v: str) -> "Value":
        """Return a new String-typed value wrapping *v*."""
        return cls(ValueType.String, str(v))

    @classmethod
    def from_struct(cls, name: str, fields: Dict[str, "Value"]) -> "Value":
        """Return a new Struct-typed value."""
        return cls(ValueType.Struct, (str(name), dict(fields)))

    @classmethod
    def from_array(cls, elements: List["Value"]) -> "Value":
        """Return a new Array-typed value."""
        return cls(ValueType.Array, list(elements))

    def as_bool(self) -> bool:
        """Return the payload as a Python ``bool``, raising ``TypeError`` if the type is not Bool."""
        if self.type != ValueType.Bool:
            raise TypeError(f"Value is {self.type.name}, not Bool")
        return bool(self._data)

    def as_int(self) -> int:
        """Return the payload as a Python ``int``, raising ``TypeError`` if the type is not Int."""
        if self.type != ValueType.Int:
            raise TypeError(f"Value is {self.type.name}, not Int")
        return int(self._data)  # type: ignore[arg-type]

    def as_float(self) -> float:
        """Return the payload as a Python ``float``, raising ``TypeError`` if the type is not Float."""
        if self.type != ValueType.Float:
            raise TypeError(f"Value is {self.type.name}, not Float")
        return float(self._data)  # type: ignore[arg-type]

    def as_string(self) -> str:
        """Return the payload as a Python ``str``, raising ``TypeError`` if the type is not String."""
        if self.type != ValueType.String:
            raise TypeError(f"Value is {self.type.name}, not String")
        return str(self._data)

    def as_struct(self) -> Tuple[str, Dict[str, "Value"]]:
        """Return the payload as a tuple of (name, fields dict), raising ``TypeError`` if the type is not Struct."""
        if self.type != ValueType.Struct:
            raise TypeError(f"Value is {self.type.name}, not Struct")
        return self._data  # type: ignore[return-value]

    def as_array(self) -> List["Value"]:
        """Return the payload as a Python ``list`` of ``Value``s, raising ``TypeError`` if the type is not Array."""
        if self.type != ValueType.Array:
            raise TypeError(f"Value is {self.type.name}, not Array")
        return self._data  # type: ignore[return-value]

    def __repr__(self) -> str:
        """Return a concise debug representation showing the type and payload."""
        if self.type == ValueType.Null:
            return "Value(null)"
        return f"Value({self.type.name}, {self._data!r})"

    def __eq__(self, other: object) -> bool:
        """Return ``True`` if *other* is a :class:`Value` with identical type and payload."""
        if not isinstance(other, Value):
            return NotImplemented
        return self.type == other.type and self._data == other._data