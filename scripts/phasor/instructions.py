"""
phasor.instructions
===================
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

    op:       OpCode
    operand1: int = 0
    operand2: int = 0
    operand3: int = 0
    operand4: int = 0
    operand5: int = 0

    @classmethod
    def halt(cls) -> "Instruction":
        return cls(OpCode.HALT)

    @classmethod
    def push_const(cls, const_index: int) -> "Instruction":
        return cls(OpCode.PUSH_CONST, const_index)

    @classmethod
    def load_var(cls, var_index: int) -> "Instruction":
        return cls(OpCode.LOAD_VAR, var_index)

    @classmethod
    def store_var(cls, var_index: int) -> "Instruction":
        return cls(OpCode.STORE_VAR, var_index)

    @classmethod
    def call(cls, name_const_index: int, arg_count: int = 0) -> "Instruction":
        return cls(OpCode.CALL, name_const_index, arg_count)

    @classmethod
    def jump(cls, offset: int) -> "Instruction":
        return cls(OpCode.JUMP, offset)

    @classmethod
    def jump_if_false(cls, offset: int) -> "Instruction":
        return cls(OpCode.JUMP_IF_FALSE, offset)

    @classmethod
    def jump_if_true(cls, offset: int) -> "Instruction":
        return cls(OpCode.JUMP_IF_TRUE, offset)

    def __repr__(self) -> str:
        ops = [self.operand1, self.operand2, self.operand3,
               self.operand4, self.operand5]
        non_zero = [str(o) for o in ops if o != 0]
        suffix = (", " + ", ".join(non_zero)) if non_zero else ""
        return f"Instruction({self.op.name}{suffix})"
