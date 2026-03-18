"""
phasor.instructions
===================
Single VM instruction as it appears both in memory and on disk.

Wire layout (21 bytes)::

    opcode   : uint8
    operand1 : int32
    operand2 : int32
    operand3 : int32
    operand4 : int32
    operand5 : int32
"""

from __future__ import annotations

from dataclasses import dataclass, field

from .opcodes import OpCode

_WIRE_SIZE = 21  # bytes on disk


@dataclass
class Instruction:
    """A single Phasor VM instruction with an :class:`~phasor.opcodes.OpCode` and up to five ``int32`` operands.

    Unused operands default to ``0``.  The on-disk representation is always
    :data:`_WIRE_SIZE` (21) bytes: one ``uint8`` opcode followed by five ``int32`` fields.
    Prefer the factory class-methods (:meth:`halt`, :meth:`push_const`, etc.) for
    common opcodes rather than constructing instances directly.
    """

    op:       OpCode
    operand1: int = 0
    operand2: int = 0
    operand3: int = 0
    operand4: int = 0
    operand5: int = 0

    @classmethod
    def halt(cls) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.HALT` instruction."""
        return cls(OpCode.HALT)

    @classmethod
    def push_const(cls, const_index: int) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.PUSH_CONST` instruction that pushes :attr:`Bytecode.constants[const_index] <phasor.bytecode.Bytecode.constants>` onto the stack."""
        return cls(OpCode.PUSH_CONST, const_index)

    @classmethod
    def load_var(cls, var_index: int) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.LOAD_VAR` instruction that pushes variable slot *var_index* onto the stack."""
        return cls(OpCode.LOAD_VAR, var_index)

    @classmethod
    def store_var(cls, var_index: int) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.STORE_VAR` instruction that pops TOS into variable slot *var_index*."""
        return cls(OpCode.STORE_VAR, var_index)

    @classmethod
    def call(cls, name_const_index: int, arg_count: int = 0) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.CALL` instruction.

        Args:
            name_const_index: Index into the constant pool where the function name string is stored.
            arg_count: Number of arguments already pushed onto the stack (default ``0``).
        """
        return cls(OpCode.CALL, name_const_index, arg_count)

    @classmethod
    def jump(cls, offset: int) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.JUMP` instruction that unconditionally transfers control to instruction *offset*."""
        return cls(OpCode.JUMP, offset)

    @classmethod
    def jump_if_false(cls, offset: int) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.JUMP_IF_FALSE` instruction that branches to *offset* and pops TOS when it is falsy."""
        return cls(OpCode.JUMP_IF_FALSE, offset)

    @classmethod
    def jump_if_true(cls, offset: int) -> "Instruction":
        """Return a :attr:`~phasor.opcodes.OpCode.JUMP_IF_TRUE` instruction that branches to *offset* and pops TOS when it is truthy."""
        return cls(OpCode.JUMP_IF_TRUE, offset)

    def __repr__(self) -> str:
        """Return a concise debug representation showing the opcode name and non-zero operands."""
        ops = [self.operand1, self.operand2, self.operand3,
               self.operand4, self.operand5]
        non_zero = [str(o) for o in ops if o != 0]
        suffix = (", " + ", ".join(non_zero)) if non_zero else ""
        return f"Instruction({self.op.name}{suffix})"
