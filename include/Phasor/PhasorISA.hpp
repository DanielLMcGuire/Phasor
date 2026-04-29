// Copyright 2026 Daniel McGuire
// Licensed under the Apache License (with LLVM-Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// https://llvm.org/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// README
//
// This file defines the Phasor VM instruction set, Bytecode format, and Instruction format.
// Use them to construct custom bytecode. See [phasor-isa(7)](phasor-docs.pages.dev/man?f=phasor-isa.7)

#pragma once

#include <cstdint>

#include <string>
#include <vector>
#include <map>

#include "../Value.hpp"

namespace Phasor
{
/**
 * @class OpCode
 * @brief Expanded opcode set for Phasor VM
 */
enum class OpCode : uint8_t
{
	// Stack operations
	PUSH_CONST, ///< Push constant from constant pool
	POP,        ///< Pop top of stack

	// Arithmetic operations
	IADD,       ///< Pop b, pop a, push a + b
	ISUBTRACT,  ///< Pop b, pop a, push a - b
	IMULTIPLY,  ///< Pop b, pop a, push a * b
	IDIVIDE,    ///< Pop b, pop a, push a / b
	IMODULO,    ///< Pop b, pop a, push a % b
	FLADD,      ///< Pop b, pop a, push a + b
	FLSUBTRACT, ///< Pop b, pop a, push a - b
	FLMULTIPLY, ///< Pop b, pop a, push a * b
	FLDIVIDE,   ///< Pop b, pop a, push a / b
	FLMODULO,   ///< Pop b, pop a, push a % b
	SQRT,       ///< sqrt()
	POW,        ///< pow()
	LOG,        ///< log()
	EXP,        ///< exp()
	SIN,        ///< sin()
	COS,        ///< cos()
	TAN,        ///< tan()

	// Unary operations
	NEGATE, ///< Pop a, push -a
	NOT,    ///< Pop a, push !a

	// Logical operations
	IAND,  ///< Pop b, pop a, push a && b
	IOR,   ///< Pop b, pop a, push a || b
	FLAND, ///< Pop b, pop a, push a && b
	FLOR,  ///< Pop b, pop a, push a || b

	// Comparison operations
	IEQUAL,          ///< Pop b, pop a, push a == b
	INOT_EQUAL,      ///< Pop b, pop a, push a != b
	ILESS_THAN,      ///< Pop b, pop a, push a < b
	IGREATER_THAN,   ///< Pop b, pop a, push a > b
	ILESS_EQUAL,     ///< Pop b, pop a, push a <= b
	IGREATER_EQUAL,  ///< Pop b, pop a, push a >= b
	FLEQUAL,         ///< Pop b, pop a, push a == b
	FLNOT_EQUAL,     ///< Pop b, pop a, push a != b
	FLLESS_THAN,     ///< Pop b, pop a, push a < b
	FLGREATER_THAN,  ///< Pop b, pop a, push a > b
	FLLESS_EQUAL,    ///< Pop b, pop a, push a <= b
	FLGREATER_EQUAL, ///< Pop b, pop a, push a >= b

	// Control flow
	JUMP,          ///< Unconditional jump to offset
	JUMP_IF_FALSE, ///< Jump if top of stack is false (pops value)
	JUMP_IF_TRUE,  ///< Jump if top of stack is true (pops value)
	JUMP_BACK,     ///< Jump backwards (for loops)

	// Variable operations
	STORE_VAR, ///< Pop top of stack, store in variable slot
	LOAD_VAR,  ///< Push variable value onto stack

	// I/O and control
	PRINT,       ///< Pop top of stack and print
	PRINTERROR,  ///< Pop top of stack and print to stderr
	READLINE,    ///< Read line from input and push onto stack
	IMPORT,      ///< Import a module: operand is index of module path in constants
	HALT,        ///< Stop execution
	CALL_NATIVE, ///< Call a native function: operand is index of function name in constants
	CALL,        ///< Call a user function: operand is index of function name in constants
	SYSTEM,      ///< Call a system function: operand is index of function name in constants
	SYSTEM_OUT,  ///< Call system function and push stdout
	SYSTEM_ERR,  ///< Call system function and push stderr
	RETURN,      ///< Return from function

	// Literal values
	TRUE_P,   ///< Push true
	FALSE_P,  ///< Push false
	NULL_VAL, ///< Push null

	// String operations
	LEN,     ///< Pop s, push len(s)
	CHAR_AT, ///< Pop index, pop s, push s[index]
	SUBSTR,  ///< Pop len, pop start, pop s, push s.substr(start, len)

	NEW_STRUCT, ///< Create new struct: operand is index of struct name in constants
	GET_FIELD,  ///< Pop struct, pop field name, push field value
	SET_FIELD,  ///< Pop struct, pop field name, pop value, set field value

	NEW_STRUCT_INSTANCE_STATIC, ///< Create new struct instance using struct section metadata (structIndex)
	GET_FIELD_STATIC,           ///< Pop struct instance, push field by static offset (structIndex, fieldOffset)
	SET_FIELD_STATIC,           ///< Pop value and struct instance, set field by static offset

	// Register operations
	// Data movement
	MOV,          ///< Copy register to register: R[rA] = R[rB]
	LOAD_CONST_R, ///< Load constant to register: R[rA] = constants[immediate]
	LOAD_VAR_R,   ///< Load variable to register: R[rA] = variables[immediate]
	STORE_VAR_R,  ///< Store register to varible: variables[immediate] = R[rA]
	PUSH_R,       ///< Push register to stack: push(R[rA])
	PUSH2_R,      ///< Push 2 registers to stack: push2(R[rA], R[rB])
	POP_R,        ///< Pop stack to register: R[rA] = pop()
	POP2_R,       ///< Pop 2 values from stack to registers: pop2(R[rA], R[rB])

	// Register arithmetic (3-address code)
	IADD_R,  ///< R[rA] = R[rB] + R[rC]
	ISUB_R,  ///< R[rA] = R[rB] - R[rC]
	IMUL_R,  ///< R[rA] = R[rB] * R[rC]
	IDIV_R,  ///< R[rA] = R[rB] / R[rC]
	IMOD_R,  ///< R[rA] = R[rB] % R[rC]
	FLADD_R, ///< R[rA] = R[rB] + R[rC]
	FLSUB_R, ///< R[rA] = R[rB] - R[rC]
	FLMUL_R, ///< R[rA] = R[rB] * R[rC]
	FLDIV_R, ///< R[rA] = R[rB] / R[rC]
	FLMOD_R, ///< R[rA] = R[rB] % R[rC]
	SQRT_R,  ///< R[rA] = sqrt(R[rB])
	POW_R,   ///< R[rA] = pow(R[rB], R[rC])
	LOG_R,   ///< R[rA] = log(R[rB])
	EXP_R,   ///< R[rA] = exp(R[rB])
	SIN_R,   ///< R[rA] = sin(R[rB])
	COS_R,   ///< R[rA] = cos(R[rB])
	TAN_R,   ///< R[rA] = tan(R[rB])

	// Register comparisons
	IAND_R,  ///< R[rA] = R[rB] && R[rC]
	IOR_R,   ///< R[rA] = R[rB] || R[rC]
	IEQ_R,   ///< R[rA] = R[rB] == R[rC]
	INE_R,   ///< R[rA] = R[rB] != R[rC]
	ILT_R,   ///< R[rA] = R[rB] < R[rC]
	IGT_R,   ///< R[rA] = R[rB] > R[rC]
	ILE_R,   ///< R[rA] = R[rB] <= R[rC]
	IGE_R,   ///< R[rA] = R[rB] >= R[rC]
	FLAND_R, ///< R[rA] = R[rB] && R[rC]
	FLOR_R,  ///< R[rA] = R[rB] || R[rC]
	FLEQ_R,  ///< R[rA] = R[rB] == R[rC]
	FLNE_R,  ///< R[rA] = R[rB] != R[rC]
	FLLT_R,  ///< R[rA] = R[rB] < R[rC]
	FLGT_R,  ///< R[rA] = R[rB] > R[rC]
	FLLE_R,  ///< R[rA] = R[rB] <= R[rC]
	FLGE_R,  ///< R[rA] = R[rB] >= R[rC]

	// Register unary operations
	NEG_R, ///< R[rA] = -R[rB]
	NOT_R, ///< R[rA] = !R[rB]

	// Register I/O
	PRINT_R,      ///< Print register: print(R[rA])
	PRINTERROR_R, ///< Print register to stderr: printerror(R[rA])
	READLINE_R,   ///< Read line into register: readline(R[rA])
	SYSTEM_R,     ///< Run an operating system shell command: system(R[rA])
	SYSTEM_OUT_R, /// Run shell command and get output: system_out(R[rA], R[rB])
	SYSTEM_ERR_R  /// Run shell command and get error output: system_err(R[rA], R[rB])
};

/**
 * @struct Instruction
 * @brief Single instruction in the Phasor VM
 */
struct Instruction
{
	OpCode  op;       ///< Operation code
	int32_t operand1; ///< First operand
	int32_t operand2; ///< Second operand
	int32_t operand3; ///< Third operand

	// Default constructor
	Instruction() : op(OpCode::HALT), operand1(0), operand2(0), operand3(0)
	{
	}

	// Full constructor
	Instruction(OpCode op, int32_t op1 = 0, int32_t op2 = 0, int32_t op3 = 0)
	    : op(op), operand1(op1), operand2(op2), operand3(op3)
	{
	}
};

/// @brief Struct metadata stored alongside bytecode (struct section)
struct StructInfo
{
	std::string              name;            ///< Struct name
	int                      firstConstIndex; ///< Index into constants for the first default value
	int                      fieldCount;      ///< Number of fields in this struct
	std::vector<std::string> fieldNames;      ///< Field names in declaration order
};

/// @brief Complete bytecode structure
struct Bytecode
{
	std::vector<Instruction>   instructions;     ///< List of instructions
	std::vector<Value>         constants;        ///< Constant pool
	std::map<std::string, int> variables;        ///< Variable name -> index mapping
	std::map<std::string, int> functionEntries;  ///< Function name -> instruction index mapping
	std::map<std::string, int> functionParamCounts; ///< Function name -> parameter count
	int                        nextVarIndex = 0; ///< Next available variable index

	// Struct section (planned usage by future struct codegen)
	std::vector<StructInfo>    structs;       ///< List of struct descriptors
	std::map<std::string, int> structEntries; ///< Struct name -> index in structs

	/// @brief Add a constant to the pool and return its index
	int addConstant(const Value &value)
	{
		constants.push_back(value);
		return static_cast<int>(constants.size()) - 1;
	}

	/// @brief Get or create a variable index
	int getOrCreateVar(const std::string &name)
	{
		auto it = variables.find(name);
		if (it != variables.end())
		{
			return it->second;
		}
		int index = nextVarIndex++;
		variables[name] = index;
		return index;
	}

	/// @brief Emit an instruction with operands
	void emit(OpCode op, int32_t op1 = 0, int32_t op2 = 0, int32_t op3 = 0)
	{
		instructions.push_back(Instruction(op, op1, op2, op3));
	}
};

} // namespace Phasor