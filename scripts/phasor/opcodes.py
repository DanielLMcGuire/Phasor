"""
phasor.opcodes
==============
OpCode enumeration — mirrors ISA.hpp exactly.

Values are assigned in declaration order starting at 0, matching the C++
``enum class OpCode : uint8_t`` which uses implicit sequential values.
"""

from enum import IntEnum


class OpCode(IntEnum):
    PUSH_CONST = 0x00   # Push constant from constant pool
    POP        = 0x01   # Pop top of stack

    IADD      = 0x02    # pop b, pop a → push a + b
    ISUBTRACT = 0x03    # pop b, pop a → push a - b
    IMULTIPLY = 0x04    # pop b, pop a → push a * b
    IDIVIDE   = 0x05    # pop b, pop a → push a / b
    IMODULO   = 0x06    # pop b, pop a → push a % b

    FLADD      = 0x07
    FLSUBTRACT = 0x08
    FLMULTIPLY = 0x09
    FLDIVIDE   = 0x0A
    FLMODULO   = 0x0B

    SQRT = 0x0C
    POW  = 0x0D
    LOG  = 0x0E
    EXP  = 0x0F
    SIN  = 0x10
    COS  = 0x11
    TAN  = 0x12

    NEGATE = 0x13   # pop a → push -a
    NOT    = 0x14   # pop a → push !a

    IAND  = 0x15
    IOR   = 0x16
    FLAND = 0x17
    FLOR  = 0x18

    IEQUAL         = 0x19
    INOT_EQUAL     = 0x1A
    ILESS_THAN     = 0x1B
    IGREATER_THAN  = 0x1C
    ILESS_EQUAL    = 0x1D
    IGREATER_EQUAL = 0x1E

    FLEQUAL         = 0x1F
    FLNOT_EQUAL     = 0x20
    FLLESS_THAN     = 0x21
    FLGREATER_THAN  = 0x22
    FLLESS_EQUAL    = 0x23
    FLGREATER_EQUAL = 0x24

    JUMP          = 0x25   # unconditional jump to offset
    JUMP_IF_FALSE = 0x26   # jump if TOS is false (pops value)
    JUMP_IF_TRUE  = 0x27   # jump if TOS is true  (pops value)
    JUMP_BACK     = 0x28   # backward jump (loops)

    STORE_VAR = 0x29   # pop TOS → variable slot
    LOAD_VAR  = 0x2A   # push variable value

    PRINT       = 0x2B
    PRINTERROR  = 0x2C
    READLINE    = 0x2D
    IMPORT      = 0x2E
    HALT        = 0x2F
    CALL_NATIVE = 0x30
    CALL        = 0x31
    SYSTEM      = 0x32
    SYSTEM_OUT  = 0x33
    SYSTEM_ERR  = 0x34
    RETURN      = 0x35

    TRUE_P   = 0x36
    FALSE_P  = 0x37
    NULL_VAL = 0x38

    LEN     = 0x39   # pop s → push len(s)
    CHAR_AT = 0x3A   # pop index, pop s → push s[index]
    SUBSTR  = 0x3B   # pop len, pop start, pop s → push s[start:start+len]

    NEW_STRUCT = 0x3C
    GET_FIELD  = 0x3D
    SET_FIELD  = 0x3E

    NEW_STRUCT_INSTANCE_STATIC = 0x3F
    GET_FIELD_STATIC           = 0x40
    SET_FIELD_STATIC           = 0x41

    MOV          = 0x42   # R[rA] = R[rB]
    LOAD_CONST_R = 0x43   # R[rA] = constants[imm]
    LOAD_VAR_R   = 0x44   # R[rA] = variables[imm]
    STORE_VAR_R  = 0x45   # variables[imm] = R[rA]
    PUSH_R       = 0x46   # push(R[rA])
    PUSH2_R      = 0x47   # push(R[rA]); push(R[rB])
    POP_R        = 0x48   # R[rA] = pop()
    POP2_R       = 0x49   # R[rA], R[rB] = pop2()

    IADD_R  = 0x4A   # R[rA] = R[rB] + R[rC]
    ISUB_R  = 0x4B
    IMUL_R  = 0x4C
    IDIV_R  = 0x4D
    IMOD_R  = 0x4E
    FLADD_R = 0x4F
    FLSUB_R = 0x50
    FLMUL_R = 0x51
    FLDIV_R = 0x52
    FLMOD_R = 0x53
    SQRT_R  = 0x54   # R[rA] = sqrt(R[rB])
    POW_R   = 0x55   # R[rA] = pow(R[rB], R[rC])
    LOG_R   = 0x56
    EXP_R   = 0x57
    SIN_R   = 0x58
    COS_R   = 0x59
    TAN_R   = 0x5A

    IAND_R  = 0x5B
    IOR_R   = 0x5C
    IEQ_R   = 0x5D
    INE_R   = 0x5E
    ILT_R   = 0x5F
    IGT_R   = 0x60
    ILE_R   = 0x61
    IGE_R   = 0x62
    FLAND_R = 0x63
    FLOR_R  = 0x64
    FLEQ_R  = 0x65
    FLNE_R  = 0x66
    FLLT_R  = 0x67
    FLGT_R  = 0x68
    FLLE_R  = 0x69
    FLGE_R  = 0x6A

    NEG_R = 0x6B   # R[rA] = -R[rB]
    NOT_R = 0x6C   # R[rA] = !R[rB]

    PRINT_R      = 0x6D
    PRINTERROR_R = 0x6E
    READLINE_R   = 0x6F
    SYSTEM_R     = 0x70
    SYSTEM_OUT_R = 0x71
    SYSTEM_ERR_R = 0x72
