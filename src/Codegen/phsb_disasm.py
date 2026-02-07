#!/usr/bin/env python3
"""
Phasor Disassembler
"""

import struct
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Any
from enum import IntEnum


class ValueType(IntEnum):
    """Value types in constant pool"""
    NULL = 0
    BOOL = 1
    INT = 2
    FLOAT = 3
    STRING = 4


class OpCode(IntEnum):
    """Phasor VM OpCodes"""
    PUSH_CONST = 0
    POP = 1
    IADD = 2
    ISUBTRACT = 3
    IMULTIPLY = 4
    IDIVIDE = 5
    IMODULO = 6
    FLADD = 7
    FLSUBTRACT = 8
    FLMULTIPLY = 9
    FLDIVIDE = 10
    FLMODULO = 11
    SQRT = 12
    POW = 13
    LOG = 14
    EXP = 15
    SIN = 16
    COS = 17
    TAN = 18
    NEGATE = 19
    NOT = 20
    IAND = 21
    IOR = 22
    FLAND = 23
    FLOR = 24
    IEQUAL = 25
    INOT_EQUAL = 26
    ILESS_THAN = 27
    IGREATER_THAN = 28
    ILESS_EQUAL = 29
    IGREATER_EQUAL = 30
    FLEQUAL = 31
    FLNOT_EQUAL = 32
    FLLESS_THAN = 33
    FLGREATER_THAN = 34
    FLLESS_EQUAL = 35
    FLGREATER_EQUAL = 36
    JUMP = 37
    JUMP_IF_FALSE = 38
    JUMP_IF_TRUE = 39
    JUMP_BACK = 40
    STORE_VAR = 41
    LOAD_VAR = 42
    PRINT = 43
    PRINTERROR = 44
    READLINE = 45
    IMPORT = 46
    HALT = 47
    CALL_NATIVE = 48
    CALL = 49
    SYSTEM = 50
    SYSTEM_OUT = 51
    SYSTEM_ERR = 52
    RETURN = 53
    TRUE_P = 54
    FALSE_P = 55
    NULL_VAL = 56
    LEN = 57
    CHAR_AT = 58
    SUBSTR = 59
    NEW_STRUCT = 60
    GET_FIELD = 61
    SET_FIELD = 62
    NEW_STRUCT_INSTANCE_STATIC = 63
    GET_FIELD_STATIC = 64
    SET_FIELD_STATIC = 65
    MOV = 66
    LOAD_CONST_R = 67
    LOAD_VAR_R = 68
    STORE_VAR_R = 69
    PUSH_R = 70
    PUSH2_R = 71
    POP_R = 72
    POP2_R = 73
    IADD_R = 74
    ISUB_R = 75
    IMUL_R = 76
    IDIV_R = 77
    IMOD_R = 78
    FLADD_R = 79
    FLSUB_R = 80
    FLMUL_R = 81
    FLDIV_R = 82
    FLMOD_R = 83
    SQRT_R = 84
    POW_R = 85
    LOG_R = 86
    EXP_R = 87
    SIN_R = 88
    COS_R = 89
    TAN_R = 90
    IAND_R = 91
    IOR_R = 92
    IEQ_R = 93
    INE_R = 94
    ILT_R = 95
    IGT_R = 96
    ILE_R = 97
    IGE_R = 98
    FLAND_R = 99
    FLOR_R = 100
    FLEQ_R = 101
    FLNE_R = 102
    FLLT_R = 103
    FLGT_R = 104
    FLLE_R = 105
    FLGE_R = 106
    NEG_R = 107
    NOT_R = 108
    PRINT_R = 109
    PRINTERROR_R = 110
    READLINE_R = 111
    SYSTEM_R = 112
    SYSTEM_OUT_R = 113
    SYSTEM_ERR_R = 114


# OpCode to string mapping
OPCODE_NAMES = {
    OpCode.PUSH_CONST: "PUSH_CONST",
    OpCode.POP: "POP",
    OpCode.IADD: "IADD",
    OpCode.ISUBTRACT: "ISUBTRACT",
    OpCode.IMULTIPLY: "IMULTIPLY",
    OpCode.IDIVIDE: "IDIVIDE",
    OpCode.IMODULO: "IMODULO",
    OpCode.FLADD: "FLADD",
    OpCode.FLSUBTRACT: "FLSUBTRACT",
    OpCode.FLMULTIPLY: "FLMULTIPLY",
    OpCode.FLDIVIDE: "FLDIVIDE",
    OpCode.FLMODULO: "FLMODULO",
    OpCode.SQRT: "SQRT",
    OpCode.POW: "POW",
    OpCode.LOG: "LOG",
    OpCode.EXP: "EXP",
    OpCode.SIN: "SIN",
    OpCode.COS: "COS",
    OpCode.TAN: "TAN",
    OpCode.NEGATE: "NEGATE",
    OpCode.NOT: "NOT",
    OpCode.IAND: "IAND",
    OpCode.IOR: "IOR",
    OpCode.IEQUAL: "IEQUAL",
    OpCode.INOT_EQUAL: "INOT_EQUAL",
    OpCode.ILESS_THAN: "ILESS_THAN",
    OpCode.IGREATER_THAN: "IGREATER_THAN",
    OpCode.ILESS_EQUAL: "ILESS_EQUAL",
    OpCode.IGREATER_EQUAL: "IGREATER_EQUAL",
    OpCode.FLAND: "FLAND",
    OpCode.FLOR: "FLOR",
    OpCode.FLEQUAL: "FLEQUAL",
    OpCode.FLNOT_EQUAL: "FLNOT_EQUAL",
    OpCode.FLLESS_THAN: "FLLESS_THAN",
    OpCode.FLGREATER_THAN: "FLGREATER_THAN",
    OpCode.FLLESS_EQUAL: "FLLESS_EQUAL",
    OpCode.FLGREATER_EQUAL: "FLGREATER_EQUAL",
    OpCode.JUMP: "JUMP",
    OpCode.JUMP_IF_FALSE: "JUMP_IF_FALSE",
    OpCode.JUMP_IF_TRUE: "JUMP_IF_TRUE",
    OpCode.JUMP_BACK: "JUMP_BACK",
    OpCode.STORE_VAR: "STORE_VAR",
    OpCode.LOAD_VAR: "LOAD_VAR",
    OpCode.PRINT: "PRINT",
    OpCode.PRINTERROR: "PRINTERROR",
    OpCode.READLINE: "READLINE",
    OpCode.IMPORT: "IMPORT",
    OpCode.HALT: "HALT",
    OpCode.CALL_NATIVE: "CALL_NATIVE",
    OpCode.CALL: "CALL",
    OpCode.SYSTEM: "SYSTEM",
    OpCode.SYSTEM_OUT: "SYSTEM_OUT",
    OpCode.SYSTEM_ERR: "SYSTEM_ERR",
    OpCode.RETURN: "RETURN",
    OpCode.TRUE_P: "TRUE",
    OpCode.FALSE_P: "FALSE",
    OpCode.NULL_VAL: "NULL_VAL",
    OpCode.LEN: "LEN",
    OpCode.CHAR_AT: "CHAR_AT",
    OpCode.SUBSTR: "SUBSTR",
    OpCode.NEW_STRUCT: "NEW_STRUCT",
    OpCode.GET_FIELD: "GET_FIELD",
    OpCode.SET_FIELD: "SET_FIELD",
    OpCode.NEW_STRUCT_INSTANCE_STATIC: "NEW_STRUCT_INSTANCE_STATIC",
    OpCode.GET_FIELD_STATIC: "GET_FIELD_STATIC",
    OpCode.SET_FIELD_STATIC: "SET_FIELD_STATIC",
    OpCode.MOV: "MOV",
    OpCode.LOAD_CONST_R: "LOAD_CONST_R",
    OpCode.LOAD_VAR_R: "LOAD_VAR_R",
    OpCode.STORE_VAR_R: "STORE_VAR_R",
    OpCode.PUSH_R: "PUSH_R",
    OpCode.PUSH2_R: "PUSH2_R",
    OpCode.POP_R: "POP_R",
    OpCode.POP2_R: "POP2_R",
    OpCode.IADD_R: "IADD_R",
    OpCode.ISUB_R: "ISUB_R",
    OpCode.IMUL_R: "IMUL_R",
    OpCode.IDIV_R: "IDIV_R",
    OpCode.IMOD_R: "IMOD_R",
    OpCode.FLADD_R: "FLADD_R",
    OpCode.FLSUB_R: "FLSUB_R",
    OpCode.FLMUL_R: "FLMUL_R",
    OpCode.FLDIV_R: "FLDIV_R",
    OpCode.FLMOD_R: "FLMOD_R",
    OpCode.SQRT_R: "SQRT_R",
    OpCode.POW_R: "POW_R",
    OpCode.LOG_R: "LOG_R",
    OpCode.EXP_R: "EXP_R",
    OpCode.SIN_R: "SIN_R",
    OpCode.COS_R: "COS_R",
    OpCode.TAN_R: "TAN_R",
    OpCode.IAND_R: "IAND_R",
    OpCode.IOR_R: "IOR_R",
    OpCode.IEQ_R: "IEQ_R",
    OpCode.INE_R: "INE_R",
    OpCode.ILT_R: "ILT_R",
    OpCode.IGT_R: "IGT_R",
    OpCode.ILE_R: "ILE_R",
    OpCode.IGE_R: "IGE_R",
    OpCode.FLAND_R: "FLAND_R",
    OpCode.FLOR_R: "FLOR_R",
    OpCode.FLEQ_R: "FLEQ_R",
    OpCode.FLNE_R: "FLNE_R",
    OpCode.FLLT_R: "FLLT_R",
    OpCode.FLGT_R: "FLGT_R",
    OpCode.FLLE_R: "FLLE_R",
    OpCode.FLGE_R: "FLGE_R",
    OpCode.NEG_R: "NEG_R",
    OpCode.NOT_R: "NOT_R",
    OpCode.PRINT_R: "PRINT_R",
    OpCode.PRINTERROR_R: "PRINTERROR_R",
    OpCode.READLINE_R: "READLINE_R",
    OpCode.SYSTEM_R: "SYSTEM_R",
    OpCode.SYSTEM_OUT_R: "SYSTEM_OUT_R",
    OpCode.SYSTEM_ERR_R: "SYSTEM_ERR_R",
}

# Section IDs
SECTION_CONSTANTS = 0x01
SECTION_VARIABLES = 0x02
SECTION_INSTRUCTIONS = 0x03
SECTION_FUNCTIONS = 0x04

# File format constants
MAGIC_NUMBER = 0x42534850  # 'PHSB' in little endian
VERSION = 0x03000000       # Version 3.0.0.0


class OperandType(IntEnum):
    NONE = 0
    INT = 1
    REGISTER = 2
    CONSTANT_IDX = 3
    VARIABLE_IDX = 4
    FUNCTION_IDX = 5


def get_operand_count(op: OpCode) -> int:
    """Get number of operands for an opcode"""
    zero_operand_ops = {
        OpCode.POP, OpCode.IADD, OpCode.ISUBTRACT, OpCode.IMULTIPLY, OpCode.IDIVIDE,
        OpCode.IMODULO, OpCode.FLADD, OpCode.FLSUBTRACT, OpCode.FLMULTIPLY, OpCode.FLDIVIDE,
        OpCode.FLMODULO, OpCode.SQRT, OpCode.POW, OpCode.LOG, OpCode.EXP, OpCode.SIN,
        OpCode.COS, OpCode.TAN, OpCode.NEGATE, OpCode.NOT, OpCode.IAND, OpCode.IOR,
        OpCode.IEQUAL, OpCode.INOT_EQUAL, OpCode.ILESS_THAN, OpCode.IGREATER_THAN,
        OpCode.ILESS_EQUAL, OpCode.IGREATER_EQUAL, OpCode.FLEQUAL, OpCode.FLNOT_EQUAL,
        OpCode.FLLESS_THAN, OpCode.FLGREATER_THAN, OpCode.FLLESS_EQUAL, OpCode.FLGREATER_EQUAL,
        OpCode.PRINT, OpCode.PRINTERROR, OpCode.READLINE, OpCode.HALT, OpCode.RETURN,
        OpCode.TRUE_P, OpCode.FALSE_P, OpCode.NULL_VAL, OpCode.LEN, OpCode.CHAR_AT, OpCode.SUBSTR,
        OpCode.FLAND, OpCode.FLOR
    }
    
    one_operand_ops = {
        OpCode.PUSH_CONST, OpCode.JUMP, OpCode.JUMP_IF_FALSE, OpCode.JUMP_IF_TRUE,
        OpCode.JUMP_BACK, OpCode.STORE_VAR, OpCode.LOAD_VAR, OpCode.IMPORT,
        OpCode.CALL_NATIVE, OpCode.CALL, OpCode.SYSTEM, OpCode.SYSTEM_OUT, OpCode.SYSTEM_ERR,
        OpCode.PUSH_R, OpCode.POP_R, OpCode.PRINT_R, OpCode.PRINTERROR_R, OpCode.READLINE_R,
        OpCode.SYSTEM_R, OpCode.SYSTEM_OUT_R, OpCode.SYSTEM_ERR_R, OpCode.NEW_STRUCT,
        OpCode.GET_FIELD, OpCode.SET_FIELD, OpCode.NEW_STRUCT_INSTANCE_STATIC
    }
    
    two_operand_ops = {
        OpCode.MOV, OpCode.LOAD_CONST_R, OpCode.LOAD_VAR_R, OpCode.STORE_VAR_R,
        OpCode.SQRT_R, OpCode.LOG_R, OpCode.EXP_R, OpCode.SIN_R, OpCode.COS_R, OpCode.TAN_R,
        OpCode.NEG_R, OpCode.NOT_R, OpCode.PUSH2_R, OpCode.POP2_R,
        OpCode.GET_FIELD_STATIC, OpCode.SET_FIELD_STATIC
    }
    
    three_operand_ops = {
        OpCode.IADD_R, OpCode.ISUB_R, OpCode.IMUL_R, OpCode.IDIV_R, OpCode.IMOD_R,
        OpCode.FLADD_R, OpCode.FLSUB_R, OpCode.FLMUL_R, OpCode.FLDIV_R, OpCode.FLMOD_R,
        OpCode.POW_R, OpCode.IAND_R, OpCode.IOR_R, OpCode.IEQ_R, OpCode.INE_R,
        OpCode.ILT_R, OpCode.IGT_R, OpCode.ILE_R, OpCode.IGE_R, OpCode.FLAND_R,
        OpCode.FLOR_R, OpCode.FLEQ_R, OpCode.FLNE_R, OpCode.FLLT_R, OpCode.FLGT_R,
        OpCode.FLLE_R, OpCode.FLGE_R
    }
    
    if op in zero_operand_ops:
        return 0
    elif op in one_operand_ops:
        return 1
    elif op in two_operand_ops:
        return 2
    elif op in three_operand_ops:
        return 3
    else:
        return 0


def get_operand_type(op: OpCode, operand_index: int) -> OperandType:
    """Get type of operand for an opcode at given index"""
    # Stack ops
    if op == OpCode.PUSH_CONST and operand_index == 0:
        return OperandType.CONSTANT_IDX
    if op == OpCode.STORE_VAR and operand_index == 0:
        return OperandType.VARIABLE_IDX
    if op == OpCode.LOAD_VAR and operand_index == 0:
        return OperandType.VARIABLE_IDX
    if op == OpCode.IMPORT and operand_index == 0:
        return OperandType.CONSTANT_IDX
    if op == OpCode.CALL_NATIVE and operand_index == 0:
        return OperandType.CONSTANT_IDX
    if op == OpCode.CALL and operand_index == 0:
        return OperandType.FUNCTION_IDX
    if op == OpCode.SYSTEM and operand_index == 0:
        return OperandType.CONSTANT_IDX
    
    # Register ops
    if op == OpCode.LOAD_CONST_R:
        return OperandType.REGISTER if operand_index == 0 else OperandType.CONSTANT_IDX
    if op == OpCode.LOAD_VAR_R:
        return OperandType.REGISTER if operand_index == 0 else OperandType.VARIABLE_IDX
    if op == OpCode.STORE_VAR_R:
        return OperandType.VARIABLE_IDX if operand_index == 0 else OperandType.REGISTER
    
    # Registeronly ops
    if op in {OpCode.MOV, OpCode.NEG_R, OpCode.NOT_R, OpCode.SQRT_R, OpCode.LOG_R,
              OpCode.EXP_R, OpCode.SIN_R, OpCode.COS_R, OpCode.TAN_R}:
        return OperandType.REGISTER
    
    if op in {OpCode.PUSH_R, OpCode.POP_R, OpCode.PRINT_R, OpCode.PRINTERROR_R,
              OpCode.READLINE_R, OpCode.SYSTEM_R}:
        return OperandType.REGISTER
    
    if op in {OpCode.PUSH2_R, OpCode.POP2_R, OpCode.SYSTEM_OUT_R, OpCode.SYSTEM_ERR_R}:
        return OperandType.REGISTER
    
    # Threeregister op
    if op in {OpCode.IADD_R, OpCode.ISUB_R, OpCode.IMUL_R, OpCode.IDIV_R, OpCode.IMOD_R,
              OpCode.FLADD_R, OpCode.FLSUB_R, OpCode.FLMUL_R, OpCode.FLDIV_R, OpCode.FLMOD_R,
              OpCode.POW_R, OpCode.IAND_R, OpCode.IOR_R, OpCode.IEQ_R, OpCode.INE_R,
              OpCode.ILT_R, OpCode.IGT_R, OpCode.ILE_R, OpCode.IGE_R, OpCode.FLAND_R,
              OpCode.FLOR_R, OpCode.FLEQ_R, OpCode.FLNE_R, OpCode.FLLT_R, OpCode.FLGT_R,
              OpCode.FLLE_R, OpCode.FLGE_R}:
        return OperandType.REGISTER
    
    return OperandType.INT


class Instruction:
    """Represents a single bytecode instruction"""
    def __init__(self, opcode: OpCode, op1: int = 0, op2: int = 0, 
                 op3: int = 0, op4: int = 0, op5: int = 0):
        self.opcode = opcode
        self.operands = [op1, op2, op3, op4, op5]


class PhasorBinaryReader:
    """Reads Phasor bytecode binary files"""
    
    def __init__(self):
        self.data: bytes = b''
        self.pos: int = 0
        self.constants: List[Any] = []
        self.variables: Dict[str, int] = {}
        self.next_var_index: int = 0
        self.function_entries: Dict[str, int] = {}
        self.instructions: List[Instruction] = []
        
        # Reverse mappings
        self.var_names: Dict[int, str] = {}
        self.func_addresses: Dict[int, str] = {}
    
    def read_uint8(self) -> int:
        if self.pos >= len(self.data):
            raise ValueError("Unexpected end of file")
        val = self.data[self.pos]
        self.pos += 1
        return val
    
    def read_uint16(self) -> int:
        val = struct.unpack('<H', self.data[self.pos:self.pos+2])[0]
        self.pos += 2
        return val
    
    def read_uint32(self) -> int:
        val = struct.unpack('<I', self.data[self.pos:self.pos+4])[0]
        self.pos += 4
        return val
    
    def read_int32(self) -> int:
        val = struct.unpack('<i', self.data[self.pos:self.pos+4])[0]
        self.pos += 4
        return val
    
    def read_int64(self) -> int:
        val = struct.unpack('<q', self.data[self.pos:self.pos+8])[0]
        self.pos += 8
        return val
    
    def read_double(self) -> float:
        val = struct.unpack('<d', self.data[self.pos:self.pos+8])[0]
        self.pos += 8
        return val
    
    def read_string(self) -> str:
        length = self.read_uint16()
        val = self.data[self.pos:self.pos+length].decode('utf-8')
        self.pos += length
        return val
    
    def calculate_crc32(self, data: bytes) -> int:
        crc = 0xFFFFFFFF
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 1:
                    crc = (crc >> 1) ^ 0xEDB88320
                else:
                    crc >>= 1
        return crc ^ 0xFFFFFFFF
    
    def read_header(self):
        magic = self.read_uint32()
        if magic != MAGIC_NUMBER:
            raise ValueError(f"Invalid magic number: 0x{magic:08X}")
        
        version = self.read_uint32()
        if version != VERSION:
            raise ValueError(f"Incompatible version: 0x{version:08X}")
        
        flags = self.read_uint32()
        checksum = self.read_uint32()
        return checksum
    
    def read_constant_pool(self):
        section_id = self.read_uint8()
        if section_id != SECTION_CONSTANTS:
            raise ValueError(f"Expected constants section")
        
        count = self.read_uint32()
        self.constants = []
        
        for _ in range(count):
            type_tag = self.read_uint8()
            
            if type_tag == ValueType.NULL:
                self.constants.append(None)
            elif type_tag == ValueType.BOOL:
                val = self.read_uint8()
                self.constants.append(bool(val))
            elif type_tag == ValueType.INT:
                val = self.read_int64()
                self.constants.append(val)
            elif type_tag == ValueType.FLOAT:
                val = self.read_double()
                self.constants.append(val)
            elif type_tag == ValueType.STRING:
                val = self.read_string()
                self.constants.append(val)
            else:
                raise ValueError(f"Unknown value type: {type_tag}")
    
    def read_variable_mapping(self):
        section_id = self.read_uint8()
        if section_id != SECTION_VARIABLES:
            raise ValueError(f"Expected variables section")
        
        count = self.read_uint32()
        self.next_var_index = self.read_int32()
        self.variables = {}
        self.var_names = {}
        
        for _ in range(count):
            name = self.read_string()
            index = self.read_int32()
            self.variables[name] = index
            self.var_names[index] = name
    
    def read_function_entries(self):
        section_id = self.read_uint8()
        if section_id != SECTION_FUNCTIONS:
            raise ValueError(f"Expected functions section")
        
        count = self.read_uint32()
        self.function_entries = {}
        self.func_addresses = {}
        
        for _ in range(count):
            name = self.read_string()
            address = self.read_int32()
            self.function_entries[name] = address
            self.func_addresses[address] = name
    
    def read_instructions(self):
        section_id = self.read_uint8()
        if section_id != SECTION_INSTRUCTIONS:
            raise ValueError(f"Expected instructions section")
        
        count = self.read_uint32()
        self.instructions = []
        
        for _ in range(count):
            opcode = OpCode(self.read_uint8())
            op1 = self.read_int32()
            op2 = self.read_int32()
            op3 = self.read_int32()
            op4 = self.read_int32()
            op5 = self.read_int32()
            self.instructions.append(Instruction(opcode, op1, op2, op3, op4, op5))
    
    def load_file(self, filename: str):
        with open(filename, 'rb') as f:
            self.data = f.read()
        
        self.pos = 0
        expected_checksum = self.read_header()
        data_start = self.pos
        actual_checksum = self.calculate_crc32(self.data[data_start:])
        
        if actual_checksum != expected_checksum:
            raise ValueError(f"Checksum mismatch")
        
        self.read_constant_pool()
        self.read_variable_mapping()
        self.read_function_entries()
        self.read_instructions()


def escape_string(s: str) -> str:
    """Escape special characters in string"""
    s = s.replace('\\', '\\\\')
    s = s.replace('"', '\\"')
    s = s.replace('\n', '\\n')
    s = s.replace('\t', '\\t')
    s = s.replace('\r', '\\r')
    return s


def convert_to_phir(reader: PhasorBinaryReader) -> str:
    """Convert loaded bytecode to PHIR text format"""
    lines = []
    
    # header
    lines.append(".PHIR 3.0.0")
    
    # consts
    lines.append(f".CONSTANTS {len(reader.constants)}")
    for const in reader.constants:
        if const is None:
            lines.append("NULL")
        elif isinstance(const, bool):
            lines.append(f"BOOL {str(const).lower()}")
        elif isinstance(const, int):
            lines.append(f"INT {const}")
        elif isinstance(const, float):
            lines.append(f"FLOAT {const}")
        elif isinstance(const, str):
            lines.append(f'STRING "{escape_string(const)}"')
    
    # vars
    lines.append(f".VARIABLES {len(reader.variables)} {reader.next_var_index}")
    for name, index in sorted(reader.variables.items(), key=lambda x: x[1]):
        lines.append(f"{name} {index}")
    
    # funcs
    if reader.function_entries:
        lines.append(f".FUNCTIONS {len(reader.function_entries)}")
        for name, address in sorted(reader.function_entries.items(), key=lambda x: x[1]):
            lines.append(f"{name} {address}")
    
    # instructions
    lines.append(f".INSTRUCTIONS {len(reader.instructions)}")
    for instr in reader.instructions:
        op = instr.opcode
        op_name = OPCODE_NAMES.get(op, f"UNKNOWN_{op}")
        operand_count = get_operand_count(op)
        
        line_parts = [op_name]
        comment = ""
        
        for i in range(operand_count):
            operand_val = instr.operands[i]
            operand_type = get_operand_type(op, i)
            
            if operand_type == OperandType.REGISTER:
                line_parts.append(f"r{operand_val}")
            elif operand_type == OperandType.CONSTANT_IDX:
                line_parts.append(str(operand_val))
                if 0 <= operand_val < len(reader.constants):
                    val = reader.constants[operand_val]
                    if isinstance(val, str):
                        preview = val[:20] + "..." if len(val) > 20 else val
                        comment = f'const[{operand_val}]="{escape_string(preview)}"'
                    elif isinstance(val, int):
                        comment = f'const[{operand_val}]={val}'
            elif operand_type == OperandType.VARIABLE_IDX:
                line_parts.append(str(operand_val))
                if operand_val in reader.var_names:
                    comment = f"var={reader.var_names[operand_val]}"
            elif operand_type == OperandType.FUNCTION_IDX:
                line_parts.append(str(operand_val))
                if operand_val in reader.func_addresses:
                    comment = f"func={reader.func_addresses[operand_val]}"
            else:
                line_parts.append(str(operand_val))
        
        if len(line_parts) > 1:
            line = line_parts[0] + " " + ", ".join(line_parts[1:])
        else:
            line = line_parts[0]
        
        if comment:
            if len(line) < 40:
                line += " " * (40 - len(line))
            else:
                line += " "
            line += f"; {comment}"
        
        lines.append(line)
    
    return "\n".join(lines)


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {Path(__file__).stem} <input.phsb> [output.phir]")
        print("  Converts Phasor binary (.phsb) to IR text format (.phir)")
        print("  If output file is not specified, prints to stdout")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    try:
        reader = PhasorBinaryReader()
        reader.load_file(input_file)
        phir_text = convert_to_phir(reader)
        
        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(phir_text)
            print(f"Converted {input_file} -> {output_file}")
            print(f"  Constants:   {len(reader.constants)}")
            print(f"  Variables:   {len(reader.variables)}")
            print(f"  Functions:   {len(reader.function_entries)}")
            print(f"  Instructions: {len(reader.instructions)}")
        else:
            print(phir_text)
    
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
