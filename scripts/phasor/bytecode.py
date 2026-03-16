"""
phasor.bytecode
===============
in-memory representation of a compiled
Phasor program.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional

from .instructions import Instruction
from .opcodes import OpCode
from .value import Value


@dataclass
class Bytecode:
    """
    Attributes
    ----------
    instructions:
        List of VM instructions.
    constants:
        Constant pool (indexed by ``PUSH_CONST`` / ``LOAD_CONST_R``).
    variables:
        Variable name → slot-index mapping.
    function_entries:
        Function name → instruction index (entry point).
    next_var_index:
        Next free variable slot (serialised as part of the variables section).
    """

    instructions:    List[Instruction]       = field(default_factory=list)
    constants:       List[Value]             = field(default_factory=list)
    variables:       Dict[str, int]          = field(default_factory=dict)
    function_entries: Dict[str, int]         = field(default_factory=dict)
    next_var_index:  int                     = 0

    def add_constant(self, value: Value) -> int:
        """Append *value* to the constant pool and return its index."""
        index = len(self.constants)
        self.constants.append(value)
        return index

    def find_or_add_constant(self, value: Value) -> int:
        """Return the index of an existing equal constant, or add a new one."""
        try:
            return self.constants.index(value)
        except ValueError:
            return self.add_constant(value)

    def get_or_create_var(self, name: str) -> int:
        """Return the slot index for *name*, creating it if absent."""
        if name not in self.variables:
            self.variables[name] = self.next_var_index
            self.next_var_index += 1
        return self.variables[name]

    def emit(self, op: OpCode,
             op1: int = 0, op2: int = 0, op3: int = 0,
             op4: int = 0, op5: int = 0) -> int:
        """Append an instruction and return its index."""
        self.instructions.append(Instruction(op, op1, op2, op3, op4, op5))
        return len(self.instructions) - 1

    def patch_operand1(self, instr_index: int, value: int) -> None:
        """Overwrite ``operand1`` of the instruction at *instr_index*."""
        self.instructions[instr_index].operand1 = value

    def save(self, path: Path | str) -> None:
        """Serialise and write to a ``.phsb`` file."""
        from .serializer import BytecodeSerializer
        BytecodeSerializer().save_to_file(self, Path(path))

    @classmethod
    def load(cls, path: Path | str) -> "Bytecode":
        """Deserialise from a ``.phsb`` file."""
        from .deserializer import BytecodeDeserializer
        return BytecodeDeserializer().load_from_file(Path(path))

    @classmethod
    def from_bytes(cls, data: bytes | bytearray) -> "Bytecode":
        """Deserialise from a raw byte buffer."""
        from .deserializer import BytecodeDeserializer
        return BytecodeDeserializer().deserialize(bytes(data))

    def to_bytes(self) -> bytes:
        """Serialise to a raw byte buffer."""
        from .serializer import BytecodeSerializer
        return bytes(BytecodeSerializer().serialize(self))

    @classmethod
    def from_native_binary(cls, path: Path | str) -> "Bytecode":
        from .native import extract_phsb_bytes
        raw = extract_phsb_bytes(Path(path))
        return cls.from_bytes(raw)

    def disassemble(self) -> str:
        """Return a human-readable disassembly of the instruction list."""
        lines: List[str] = []
        for i, instr in enumerate(self.instructions):
            for name, addr in self.function_entries.items():
                if addr == i:
                    lines.append(f"\n<function {name}>:")
            ops = [instr.operand1, instr.operand2, instr.operand3,
                   instr.operand4, instr.operand5]
            non_zero = [str(o) for o in ops if o != 0]
            operands = ("  " + ", ".join(non_zero)) if non_zero else ""
            lines.append(f"  {i:>4}  {instr.op.name:<20}{operands}")
        return "\n".join(lines)

    def __repr__(self) -> str:
        return (
            f"Bytecode("
            f"{len(self.instructions)} instructions, "
            f"{len(self.constants)} constants, "
            f"{len(self.variables)} variables, "
            f"{len(self.function_entries)} functions)"
        )
