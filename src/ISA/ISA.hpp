#pragma once
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

	// String operatoins
	LEN,     ///< Pop s, push len(s)
	CHAR_AT, ///< Pop index, pop s, push s[index]
	SUBSTR,  ///< Pop len, pop start, pop s, push s.substr(start, len)

	NEW_STRUCT, ///< Create new struct: operand is index of struct name in constants
	GET_FIELD,  ///< Pop struct, pop field name, push field value
	SET_FIELD,  ///< Pop struct, pop field name, pop value, set field value

	NEW_STRUCT_INSTANCE_STATIC, ///< Create new struct instance using struct section metadata (structIndex)
	GET_FIELD_STATIC,           ///< Pop struct instance, push field by static offset (structIndex, fieldOffset)
	SET_FIELD_STATIC,           ///< Pop value and struct instance, set field by static offset

	// Register-based operations (v2.0)
	// Data movement
	MOV,          ///< Copy register to register: R[rA] = R[rB]
	LOAD_CONST_R, ///< Load constant to register: R[rA] = constants[immediate]
	LOAD_VAR_R,   ///< Load variable to register: R[rA] = variables[immediate]
	STORE_VAR_R,  ///< Store register to variable: variables[immediate] = R[rA]
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
} // namespace Phasor