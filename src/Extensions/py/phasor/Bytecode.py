"""
phasor.Bytecode
================
in-memory representation of a compiled
Phasor program.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional

from .Instruction import Instruction
from .OpCode import OpCode
from .Value import Value


@dataclass
class Bytecode:
    """In-memory representation of a compiled Phasor program.

    Holds all data needed to run or serialise the program: the instruction
    stream, the constant pool, the variable name→slot mapping, function entry
    points, and the next free variable slot counter.

    Attributes:
        instructions: Ordered list of :class:`~phasor.Instruction.Instruction` objects forming the program.
        constants: Constant pool; entries are indexed by :attr:`~phasor.OpCode.OpCode.PUSH_CONST` / :attr:`~phasor.OpCode.OpCode.LOAD_CONST_R`.
        variables: Maps each variable name to its integer slot index.
        function_entries: Maps each function name to the index of its first instruction.
        next_var_index: Next available variable slot; serialised as part of the variables section.
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
             op1: int = 0, op2: int = 0, op3: int = 0) -> int:
        """Append a new :class:`~phasor.Instruction.Instruction` to :attr:`instructions` and return its index.

        Args:
            op: The :class:`~phasor.OpCode.OpCode` for this instruction.
            op1 … op5: Operand values; unused operands should be left as ``0``.

        Returns:
            The zero-based index of the newly appended instruction.
        """
        self.instructions.append(Instruction(op, op1, op2, op3))
        return len(self.instructions) - 1

    def patch_operand1(self, instr_index: int, value: int) -> None:
        """Overwrite ``operand1`` of the instruction at *instr_index* in-place.

        Typically used to back-patch forward-jump offsets after the jump target
        is known.

        Args:
            instr_index: Index into :attr:`instructions` of the instruction to patch.
            value: New value for ``operand1``.
        """
        self.instructions[instr_index].operand1 = value

    def save(self, path: Path | str) -> None:
        """Serialise this object and write it to a ``.phsb`` file at *path*.

        Delegates to :class:`~phasor.Serializer.BytecodeSerializer`.
        """
        from .Serializer import BytecodeSerializer
        BytecodeSerializer().save_to_file(self, Path(path))

    @classmethod
    def load(cls, path: Path | str) -> "Bytecode":
        """Read a ``.phsb`` file at *path* and return a deserialised :class:`Bytecode`.

        Delegates to :class:`~phasor.Deserializer.BytecodeDeserializer`.
        """
        from .Deserializer import BytecodeDeserializer
        return BytecodeDeserializer().load_from_file(Path(path))

    @classmethod
    def from_bytes(cls, data: bytes | bytearray) -> "Bytecode":
        """Deserialise a :class:`Bytecode` from a raw ``.phsb`` byte buffer.

        Delegates to :class:`~phasor.Deserializer.BytecodeDeserializer`.
        """
        from .Deserializer import BytecodeDeserializer
        return BytecodeDeserializer().deserialize(bytes(data))

    def to_bytes(self) -> bytes:
        """Serialise this object to a raw ``.phsb`` byte buffer.

        Delegates to :class:`~phasor.Serializer.BytecodeSerializer`.
        """
        from .Deserializer import BytecodeSerializer
        return bytes(BytecodeSerializer().serialize(self))

    @classmethod
    def from_native_binary(cls, path: Path | str) -> "Bytecode":
        """Extract and deserialise bytecode from an ELF/PE/MachO binary's ``.phsb`` section."""
        from .Native import extract_phsb_bytes
        raw = extract_phsb_bytes(Path(path))
        return cls.from_bytes(raw)

    def disassemble(self) -> str:
        """Return a human-readable disassembly of :attr:`instructions`.

        Function entry points from :attr:`function_entries` are printed as
        ``<function name>:`` labels above their first instruction.
        """
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
        """Return a summary showing instruction, constant, variable, and function counts."""
        return (
            f"Bytecode("
            f"{len(self.instructions)} instructions, "
            f"{len(self.constants)} constants, "
            f"{len(self.variables)} variables, "
            f"{len(self.function_entries)} functions)"
        )
