## Stack Operations

* `PUSH_CONST` – Push constant from constant pool
* `POP` – Pop top of stack

## Arithmetic Operations

* `IADD` – Pop b, pop a, push a + b
* `ISUBTRACT` – Pop b, pop a, push a - b
* `IMULTIPLY` – Pop b, pop a, push a * b
* `IDIVIDE` – Pop b, pop a, push a / b
* `IMODULO` – Pop b, pop a, push a % b
* `FLADD` – Pop b, pop a, push a + b
* `FLSUBTRACT` – Pop b, pop a, push a - b
* `FLMULTIPLY` – Pop b, pop a, push a * b
* `FLDIVIDE` – Pop b, pop a, push a / b
* `FLMODULO` – Pop b, pop a, push a % b
* `SQRT` – sqrt()
* `POW` – pow()
* `LOG` – log()
* `EXP` – exp()
* `SIN` – sin()
* `COS` – cos()
* `TAN` – tan()

## Unary Operations

* `NEGATE` – Pop a, push -a
* `NOT` – Pop a, push !a

## Logical Operations

* `IAND` – Pop b, pop a, push a && b
* `IOR` – Pop b, pop a, push a || b
* `FLAND` – Pop b, pop a, push a && b
* `FLOR` – Pop b, pop a, push a || b

## Comparison Operations

* `IEQUAL` – Pop b, pop a, push a == b
* `INOT_EQUAL` – Pop b, pop a, push a != b
* `ILESS_THAN` – Pop b, pop a, push a < b
* `IGREATER_THAN` – Pop b, pop a, push a > b
* `ILESS_EQUAL` – Pop b, pop a, push a <= b
* `IGREATER_EQUAL` – Pop b, pop a, push a >= b
* `FLEQUAL` – Pop b, pop a, push a == b
* `FLNOT_EQUAL` – Pop b, pop a, push a != b
* `FLLESS_THAN` – Pop b, pop a, push a < b
* `FLGREATER_THAN` – Pop b, pop a, push a > b
* `FLLESS_EQUAL` – Pop b, pop a, push a <= b
* `FLGREATER_EQUAL` – Pop b, pop a, push a >= b

## Control Flow

* `JUMP` – Unconditional jump to offset
* `JUMP_IF_FALSE` – Jump if top of stack is false (pops value)
* `JUMP_IF_TRUE` – Jump if top of stack is true (pops value)
* `JUMP_BACK` – Jump backwards (for loops)

## Variable Operations

* `STORE_VAR` – Pop top of stack, store in variable slot
* `LOAD_VAR` – Push variable value onto stack

## I/O and Control

* `PRINT` – Pop top of stack and print
* `PRINTERROR` – Pop top of stack and print to stderr
* `READLINE` – Read line from input and push onto stack
* `IMPORT` – Import module (operand: index in constants)
* `HALT` – Stop execution
* `CALL_NATIVE` – Call native function (operand: index in constants)
* `CALL` – Call user function (operand: index in constants)
* `SYSTEM` – Call system function (operand: index in constants)
* `SYSTEM_OUT` – Call system function and push stdout
* `SYSTEM_ERR` – Call system function and push stderr
* `RETURN` – Return from function

## Literal Values

* `TRUE_P` – Push true
* `FALSE_P` – Push false
* `NULL_VAL` – Push null

## String Operations

* `LEN` – Pop s, push len(s)
* `CHAR_AT` – Pop index, pop s, push s[index]
* `SUBSTR` – Pop len, pop start, pop s, push s.substr(start, len)

## Struct Operations

* `NEW_STRUCT` – Create new struct (operand: index in constants)
* `GET_FIELD` – Pop struct, pop field name, push field value
* `SET_FIELD` – Pop struct, pop field name, pop value, set field value
* `NEW_STRUCT_INSTANCE_STATIC` – Create struct instance using struct metadata
* `GET_FIELD_STATIC` – Pop instance, push field by static offset
* `SET_FIELD_STATIC` – Pop value and instance, set field by static offset

## Register-Based Operations (v2.0)

### Data Movement

* `MOV` – Copy register to register: `R[rA] = R[rB]`
* `LOAD_CONST_R` – Load constant to register: `R[rA] = constants[immediate]`
* `LOAD_VAR_R` – Load variable to register: `R[rA] = variables[immediate]`
* `STORE_VAR_R` – Store register to variable: `variables[immediate] = R[rA]`
* `PUSH_R` – Push register to stack: `push(R[rA])`
* `PUSH2_R` – Push 2 registers to stack: `push2(R[rA], R[rB])`
* `POP_R` – Pop stack to register: `R[rA] = pop()`
* `POP2_R` – Pop 2 values from stack to registers

### Register Arithmetic (3-Address Code)

* `IADD_R` – `R[rA] = R[rB] + R[rC]`
* `ISUB_R` – `R[rA] = R[rB] - R[rC]`
* `IMUL_R` – `R[rA] = R[rB] * R[rC]`
* `IDIV_R` – `R[rA] = R[rB] / R[rC]`
* `IMOD_R` – `R[rA] = R[rB] % R[rC]`
* `FLADD_R` – `R[rA] = R[rB] + R[rC]`
* `FLSUB_R` – `R[rA] = R[rB] - R[rC]`
* `FLMUL_R` – `R[rA] = R[rB] * R[rC]`
* `FLDIV_R` – `R[rA] = R[rB] / R[rC]`
* `FLMOD_R` – `R[rA] = R[rB] % R[rC]`
* `SQRT_R` – `R[rA] = sqrt(R[rB])`
* `POW_R` – `R[rA] = pow(R[rB], R[rC])`
* `LOG_R` – `R[rA] = log(R[rB])`
* `EXP_R` – `R[rA] = exp(R[rB])`
* `SIN_R` – `R[rA] = sin(R[rB])`
* `COS_R` – `R[rA] = cos(R[rB])`
* `TAN_R` – `R[rA] = tan(R[rB])`

### Register Comparisons

* `IAND_R` – `R[rA] = R[rB] && R[rC]`
* `IOR_R` – `R[rA] = R[rB] || R[rC]`
* `IEQ_R` – `R[rA] = R[rB] == R[rC]`
* `INE_R` – `R[rA] = R[rB] != R[rC]`
* `ILT_R` – `R[rA] = R[rB] < R[rC]`
* `IGT_R` – `R[rA] = R[rB] > R[rC]`
* `ILE_R` – `R[rA] = R[rB] <= R[rC]`
* `IGE_R` – `R[rA] = R[rB] >= R[rC]`
* `FLAND_R` – `R[rA] = R[rB] && R[rC]`
* `FLOR_R` – `R[rA] = R[rB] || R[rC]`
* `FLEQ_R` – `R[rA] = R[rB] == R[rC]`
* `FLNE_R` – `R[rA] = R[rB] != R[rC]`
* `FLLT_R` – `R[rA] = R[rB] < R[rC]`
* `FLGT_R` – `R[rA] = R[rB] > R[rC]`
* `FLLE_R` – `R[rA] = R[rB] <= R[rC]`
* `FLGE_R` – `R[rA] = R[rB] >= R[rC]`

### Register Unary Operations

* `NEG_R` – `R[rA] = -R[rB]`
* `NOT_R` – `R[rA] = !R[rB]`

### Register I/O

* `PRINT_R` – Print register: `print(R[rA])`
* `PRINTERROR_R` – Print register to stderr: `printerror(R[rA])`
* `READLINE_R` – Read line into register: `readline(R[rA])`
* `SYSTEM_R` – Run OS shell command: `system(R[rA])`
* `SYSTEM_OUT_R` – Run shell command and get output: `system_out(R[rA], R[rB])`
* `SYSTEM_ERR_R` – Run shell command and get error output: `system_err(R[rA], R[rB])`
