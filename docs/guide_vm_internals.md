# Phasor VM Internals

This document describes the internal architecture and operation of the Phasor Virtual Machine, including recent engine changes (v2.0) that introduce a register-based execution model alongside the traditional stack-based instructions.

## 1. Architecture (Stack + Register hybrid)

Phasor historically used a stack-based VM. Recent updates add a register-based execution mode (referred to in the code as v2.0). The VM now supports both stack-style opcodes and register-style opcodes; the compiler can emit either or both styles and the VM executes them in the same instruction stream.

### Key Components

- **Bytecode**: The executable `Bytecode` contains `instructions`, a `constants` pool, `functionEntries` (map from name to instruction index), and `nextVarIndex` used to size the variable table.
- **Value Registers**: `std::array<Value, 256> registers` — 256 virtual registers indexed `r0`..`r255` for register-based opcodes (v2.0).
- **Stack**: `std::vector<Value> stack` — used by stack-style opcodes, function calls, and certain mixed operations.
- **Operand Stack**: `std::vector<Value> operandStack` — reserved for safe register/operand shuffling (present in the implementation for internal use).
- **Call Stack**: `std::vector<int> callStack` — stores return addresses for `CALL`/`RETURN`.
- **Variables**: `std::vector<Value> variables` — flat storage of variables resolved by index at compile time.
- **Program Counter (PC)**: `size_t pc` — index into `bytecode->instructions`.
- **Native functions**: `std::map<std::string, NativeFunction> nativeFunctions` — host-registered native functions callable via `CALL_NATIVE`.

## 2. Value System

All data in Phasor is represented by the `Value` class (`src/Runtime/Value.hpp`). It is a variant type that can hold:

- `Null`
- `Bool`
- `Int` (64-bit signed)
- `Float` (64-bit double)
- `String` (std::string)

## 3. Execution & Instruction Styles

The VM runs a single instruction stream (`VM::run`). Each `Instruction` contains an `OpCode` and up to five integer operands. Operands are interpreted by each opcode; stack-style opcodes use the runtime stack, register-style opcodes operate on the `registers` array.

Execution loop (high level):

1. Fetch instruction at `pc` and increment `pc`.
2. Dispatch to `VM::operation(op, operand1, operand2, operand3, operand4, operand5)`.
3. Opcode handler interprets operands (as stack indices, register indices, constant indices, variable indices, or jump targets) and performs the action.

### Stack-style opcodes

- Examples: `PUSH_CONST`, `POP`, `ADD`, `SUBTRACT`, `MULTIPLY`, `DIVIDE`, `MODULO`, `SQRT`, `POW`, `LOG`, `EXP`, `SIN`, `COS`, `TAN`, `NEGATE`, `NOT`, `AND`, `OR`, `EQUAL`, `NOT_EQUAL`, `LESS_THAN`, `GREATER_THAN`, `LESS_EQUAL`, `GREATER_EQUAL`, `CALL`, `RETURN`, `STORE_VAR`, `LOAD_VAR`, `CALL_NATIVE`, `PRINT`, `PRINTERROR`, `READLINE`, `SYSTEM`, `IMPORT`, `HALT`, `TRUE`, `FALSE`, `NULL_VAL`, `LEN`, `CHAR_AT`, `SUBSTR`, `JUMP`, `JUMP_IF_FALSE`, `JUMP_IF_TRUE`, `JUMP_BACK`.
- `CALL_NATIVE` semantics: the VM expects the caller to push an integer `argCount` onto the stack; the VM then pops that count and pops the arguments (in reverse order) to build the `std::vector<Value>` passed to the C++ native function. The native function returns a `Value` which the VM pushes.

### Register-style opcodes (v2.0)

**Data Movement:**

- `MOV rA, rB` — copy register `rB` into `rA`.
- `LOAD_CONST_R rA, constIndex` — load a constant into `rA`.
- `LOAD_VAR_R rA, varIndex` — copy from variable into `rA`.
- `STORE_VAR_R rA, varIndex` — store register `rA` into variable `varIndex`.
- `PUSH_R rA` — push registers[rA] onto the stack.
- `PUSH2_R rA, rB` — push two registers onto the stack.
- `POP_R rA` — pop stack into registers[rA].
- `POP2_R rA, rB` — pop two values from stack into registers.

**Arithmetic (3-address form):**

- `ADD_R rA, rB, rC` where `rA = rB + rC` (same for `SUB_R`, `MUL_R`, `DIV_R`, `MOD_R`, `POW_R`).

**Unary Math:**

- `SQRT_R rA, rB`, `LOG_R rA, rB`, `EXP_R rA, rB`, `SIN_R rA, rB`, `COS_R rA, rB`, `TAN_R rA, rB` (dest `rA`, src `rB`).

**Logical/Comparison (3-address form):**

- `AND_R rA, rB, rC`, `OR_R rA, rB, rC`.
- `EQ_R`, `NE_R`, `LT_R`, `GT_R`, `LE_R`, `GE_R` — write boolean (0/1) into destination register.

**Unary Ops:**

- `NEG_R rA, rB`, `NOT_R rA, rB`.

**I/O:**

- `PRINT_R rA`, `PRINTERROR_R rA`, `READLINE_R rA`, `SYSTEM_R rA`.

**Operand Conventions:**

- For 3-address ops: `operand1 = rA (dest)`, `operand2 = rB`, `operand3 = rC`.
- For 2-operand ops: `operand1 = rA`, `operand2 = rB`.
- For load/store: `operand1 = rA`, `operand2 = index`.

**Notes:**

- Register indices are `uint8_t` (0..255).
- Arithmetic ops use optimized native implementations (e.g., `asm_add`, `asm_sub`) when both operands are integers.

## 4. Bytecode layout and helpers

- `Bytecode::constants` is an indexable pool of `Value` constants used by `PUSH_CONST` and `LOAD_CONST_R`.
- `Bytecode::instructions` is the sequential list of `Instruction` objects executed by the VM.
- `Bytecode::functionEntries` maps function names to the start instruction index used by `CALL`.
- `Bytecode::nextVarIndex` is used to size `variables` when `VM::run` begins.
- `Bytecode::structs` contains struct metadata for static field access.
- `Bytecode::structEntries` maps struct names to indices in `structs`.

## 5. VM API notes (runtime-accessible behavior)

- `registerNativeFunction(name, fn)` — register a host native function. The native `fn` has signature `Value(const std::vector<Value>& args, VM *vm)`.
- `setImportHandler(handler)` — provide a callback to resolve `IMPORT` opcodes; the handler receives a `filesystem::path`.
- `addVariable`, `freeVariable`, `setVariable`, `getVariable`, `getVariableCount` — helpers for variable management.
- `setRegister`, `freeRegister`, `getRegister`, `getRegisterCount` — helpers to inspect/manipulate VM registers from the host.
- `push`, `pop`, `peek` — stack helpers used by stack-style opcodes and for host interaction.
- `reset(resetStack, resetFunctions, resetVariables)` — resets selected VM state; by default `VM::run` clears stack, call stack, operand stack and resets registers and variable storage size.

## 6. Error Handling

Runtime errors (e.g., stack underflow, invalid constant/variable index, unknown function) throw `std::runtime_error` within the VM. These exceptions propagate to the host application (REPL/CLI) where they are typically caught and reported.

## 7. Struct Support

Structs are supported via static and dynamic field access opcodes:

**Static Field Access** (compile-time known fields):

- `NEW_STRUCT_INSTANCE_STATIC structIndex` — create struct instance using metadata.
- `GET_FIELD_STATIC rA, structIndex, fieldOffset` — load field into register.
- `SET_FIELD_STATIC rA, structIndex, fieldOffset` — store register into field.

**Dynamic Field Access** (runtime field names):

- `NEW_STRUCT structNameIndex` — create struct with runtime name.
- `GET_FIELD fieldNameIndex` — pop struct, load field by name.
- `SET_FIELD fieldNameIndex` — pop struct and value, set field by name.

Struct metadata is stored in `Bytecode::structs` and indexed by `Bytecode::structEntries`. Field names are resolved at compile time for static access, enabling efficient field operations.

## 8. Migration & Notes for compiler/runtime authors

- The VM accepts a mixed instruction stream: compiler writers may emit traditional stack-based instructions, register-based instructions, or a combination. Register-based instructions are more compact for arithmetic and reduce stack traffic.
- When adding new opcodes, take care to document operand meanings (register vs. index vs. jump target) and update `PhasorIR` string mappings so bytecode serializers can round-trip.
- `CALL_NATIVE` still uses the stack for argument passing (an integer arg count is expected on the stack followed by the arguments). Mixing register-based calling conventions is an implementation choice for the compiler; the runtime expects the argCount on the stack as implemented today.
- Structs use static field offsets determined at compile time for efficient field access.

---

Updated to reflect VM v2.0 register additions and struct support (see `src/Runtime/VM/VM.cpp`, `src/Runtime/VM/VM.hpp`).
