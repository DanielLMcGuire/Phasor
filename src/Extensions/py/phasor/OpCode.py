"""
phasor.OpCode
=============
OpCode enumeration — mirrors ISA.hpp exactly.

Values are assigned in declaration order starting at 0, matching the C++
``enum class OpCode : uint8_t`` which uses implicit sequential values.
"""

from enum import IntEnum


class OpCode(IntEnum):
    """Phasor VM instruction opcodes"""

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

    MADD       = 0x0C
    MSUBTRACT  = 0x0D
    MMULTIPLY  = 0x0E
    MDIVIDE    = 0x0F

    SQRT = 0x10
    POW  = 0x11
    LOG  = 0x12
    EXP  = 0x13
    SIN  = 0x14
    COS  = 0x15
    TAN  = 0x16

    NEGATE = 0x17   # pop a → push -a
    NOT    = 0x18   # pop a → push !a

    IAND  = 0x19
    IOR   = 0x1A
    FLAND = 0x1B
    FLOR  = 0x1C

    IEQUAL         = 0x1D
    INOT_EQUAL     = 0x1E
    ILESS_THAN     = 0x1F
    IGREATER_THAN  = 0x20
    ILESS_EQUAL    = 0x21
    IGREATER_EQUAL = 0x22

    FLEQUAL         = 0x23
    FLNOT_EQUAL     = 0x24
    FLLESS_THAN     = 0x25
    FLGREATER_THAN  = 0x26
    FLLESS_EQUAL    = 0x27
    FLGREATER_EQUAL = 0x28

    JUMP          = 0x29   # unconditional jump to offset
    JUMP_IF_FALSE = 0x2A   # jump if TOS is false (pops value)
    JUMP_IF_TRUE  = 0x2B   # jump if TOS is true  (pops value)
    JUMP_BACK     = 0x2C   # backward jump (loops)

    STORE_VAR = 0x2D   # pop TOS → variable slot
    LOAD_VAR  = 0x2E   # push variable value

    PRINT       = 0x2F
    PRINTERROR  = 0x30
    READLINE    = 0x31
    IMPORT      = 0x32
    HALT        = 0x33
    CALL_NATIVE = 0x34
    CALL        = 0x35
    SYSTEM      = 0x36
    SYSTEM_OUT  = 0x37
    SYSTEM_ERR  = 0x38
    RETURN      = 0x39

    TRUE_P   = 0x3A
    FALSE_P  = 0x3B
    NULL_VAL = 0x3C

    LEN     = 0x3D   # pop s → push len(s)
    CHAR_AT = 0x3E   # pop index, pop s → push s[index]
    SUBSTR  = 0x3F   # pop len, pop start, pop s → push s[start:start+len]

    NEW_STRUCT = 0x40
    GET_FIELD  = 0x41
    SET_FIELD  = 0x42

    NEW_STRUCT_INSTANCE_STATIC = 0x43
    GET_FIELD_STATIC           = 0x44
    SET_FIELD_STATIC           = 0x45

    MOV          = 0x46   # R[rA] = R[rB]
    LOAD_CONST_R = 0x47   # R[rA] = constants[imm]
    LOAD_VAR_R   = 0x48   # R[rA] = variables[imm]
    STORE_VAR_R  = 0x49   # variables[imm] = R[rA]
    PUSH_R       = 0x4A   # push(R[rA])
    PUSH2_R      = 0x4B   # push(R[rA]); push(R[rB])
    POP_R        = 0x4C   # R[rA] = pop()
    POP2_R       = 0x4D   # R[rA], R[rB] = pop2()

    IADD_R  = 0x4E   # R[rA] = R[rB] + R[rC]
    ISUB_R  = 0x4F
    IMUL_R  = 0x50
    IDIV_R  = 0x51
    IMOD_R  = 0x52
    FLADD_R = 0x53
    FLSUB_R = 0x54
    FLMUL_R = 0x55
    FLDIV_R = 0x56
    FLMOD_R = 0x57
    MADD_R  = 0x58
    MSUB_R  = 0x59
    MMUL_R  = 0x5A
    MDIV_R  = 0x5B
    SQRT_R  = 0x5C   # R[rA] = sqrt(R[rB])
    POW_R   = 0x5D   # R[rA] = pow(R[rB], R[rC])
    LOG_R   = 0x5E
    EXP_R   = 0x5F
    SIN_R   = 0x60
    COS_R   = 0x61
    TAN_R   = 0x62

    IAND_R  = 0x63
    IOR_R   = 0x64
    IEQ_R   = 0x65
    INE_R   = 0x66
    ILT_R   = 0x67
    IGT_R   = 0x68
    ILE_R   = 0x69
    IGE_R   = 0x6A
    FLAND_R = 0x6B
    FLOR_R  = 0x6C
    FLEQ_R  = 0x6D
    FLNE_R  = 0x6E
    FLLT_R  = 0x6F
    FLGT_R  = 0x70
    FLLE_R  = 0x71
    FLGE_R  = 0x72

    NEG_R = 0x73   # R[rA] = -R[rB]
    NOT_R = 0x74   # R[rA] = !R[rB]

    PRINT_R      = 0x75
    PRINTERROR_R = 0x76
    READLINE_R   = 0x77
    SYSTEM_R     = 0x78
    SYSTEM_OUT_R = 0x79
    SYSTEM_ERR_R = 0x7A
    EXIT_SCOPE   = 0x7B

    NEW_ARR   = 0x7C
    LOAD_ARR  = 0x7D
    STORE_ARR = 0x7E

    GET_FIELD_DYN = 0x7F