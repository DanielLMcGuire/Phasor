# Adding New Opcodes to Phasor VM

This guide outlines the process of adding new opcodes to the Phasor Virtual Machine. Opcodes are the fundamental instructions executed by the VM.

## 1. Define the OpCode

First, add the new opcode to the `OpCode` enum in `src/Codegen/CodeGen.hpp`.

```cpp
enum class OpCode : uint8_t
{
    // ... existing opcodes
    NEW_OPCODE, // < Description of what it does
    // ...
};
```

## 2. Update IR String Mapping

Update `PhasorIR::opCodeToString` and `PhasorIR::stringToOpCode` in `src/Codegen/IR/PhasorIR.cpp` to support serialization/deserialization of the new opcode.

```cpp
std::string PhasorIR::opCodeToString(OpCode op)
{
    switch (op)
    {
        // ...
        case OpCode::NEW_OPCODE: return "NEW_OPCODE";
        // ...
    }
}

OpCode PhasorIR::stringToOpCode(const std::string &str)
{
    // ...
    if (str == "NEW_OPCODE") return OpCode::NEW_OPCODE;
    // ...
}
```

## 3. Implement VM Logic (C++)

Implement the opcode logic in `VM::operation` within `src/Runtime/VM/VM.cpp`.

```cpp
void VM::operation(const OpCode &operation, const int &operand)
{
    switch (operation)
    {
        // ...
        case OpCode::NEW_OPCODE: {
            // Example: Pop two values, perform action, push result
            Value b = pop();
            Value a = pop();
            // Perform operation...
            push(result);
            break;
        }
        // ...
    }
}
```

### Register-style opcodes (v2.0)

The VM now supports register-style (3-address) opcodes in addition to stack-style opcodes. When adding register-based opcodes, follow these conventions used in `src/Runtime/VM/VM.cpp`:

- Destination register is encoded in `operand1` (as a byte), sources in `operand2`, `operand3` where applicable.
- For load/store with indices, the pattern is `operand1 = rA`, `operand2 = index` (constant index or variable index).
- Use `registers[uint8_t(operandN)]` to access registers safely.

Example (3-address add):

```cpp
case OpCode::ADD_R: {
    uint8_t rA = static_cast<uint8_t>(operand1);
    uint8_t rB = static_cast<uint8_t>(operand2);
    uint8_t rC = static_cast<uint8_t>(operand3);
    registers[rA] = Value(asm_add(registers[rB].asInt(), registers[rC].asInt()));
    break;
}
```

Example (load constant into register):

```cpp
case OpCode::LOAD_CONST_R: {
    uint8_t rA = static_cast<uint8_t>(operand1);
    int constIndex = operand2;
    if (constIndex < 0 || constIndex >= static_cast<int>(bytecode->constants.size()))
        throw std::runtime_error("Invalid constant index");
    registers[rA] = bytecode->constants[constIndex];
    break;
}
```

Notes:

- Register indices are bytes (0..255); the VM stores registers in a fixed-size `std::array<Value,256>`.
- Mixing stack and register opcodes in the same bytecode stream is allowed; be explicit in your emitted IR about which style you're using for each operation.

## 4. Implement Native Assembly (Optional but Recommended)

For performance-critical opcodes, implement the logic in assembly.

### Step 4a: Declare in `opcodes_native.h`

Add the function prototype to `src/Runtime/VM/opcodes_native.h`.

```c
extern "C" {
    // ...
    int64_t asm_new_opcode(int64_t a, int64_t b);
}
```

### Step 4b: Implement in Assembly

Implement the function in the appropriate assembly file for your architecture (e.g., `src/Runtime/VM/opcodes_windows64.asm` for Windows x64).

**Windows x64 Example (`opcodes_windows64.asm`):**

```asm
; int64_t asm_new_opcode(int64_t a, int64_t b)
; rcx = a, rdx = b
asm_new_opcode PROC
    mov rax, rcx
    add rax, rdx ; Example operation
    ret
asm_new_opcode ENDP
```

**Unix x64 Example (`opcodes_bsd64.s` / `opcodes_linux64.s`):**

```asm
# int64_t asm_new_opcode(int64_t a, int64_t b)
# rdi = a, rsi = b
.global asm_new_opcode
asm_new_opcode:
    mov %rdi, %rax
    add %rsi, %rax # Example operation
    ret
```

### Step 4c: Call from VM

Update `VM::operation` to call the native function.

```cpp
case OpCode::NEW_OPCODE: {
    Value b = pop();
    Value a = pop();
    push(asm_new_opcode(a.asInt(), b.asInt()));
    break;
}
```

## 5. Compiler Integration (If applicable)

If the opcode corresponds to a language feature, update `src/Codegen/CodeGen.cpp` to emit the new opcode.

```cpp
void CodeGenerator::visit(const NewFeatureNode* node) {
    // Generate code for children...
    bytecode.emit(OpCode::NEW_OPCODE);
}
```

## Edge Cases & Notes

- **Stack Management**: Ensure you `pop` the correct number of arguments and `push` the result. Stack underflow/overflow checks are handled by `pop()`/`push()`.
- **Type Safety**: Check `Value` types before operating if the opcode is type-sensitive.
- **Platform Differences**: When writing assembly, be mindful of calling conventions (Windows x64 vs System V AMD64).
