# Phasor Language Guide

Phasor is a dynamically typed scripting language designed to run on the Phasor VM.

## 1. Data Types

Phasor supports the following basic data types:

- **Integer**: 64-bit signed integers (e.g., `42`, `-10`)
- **Float**: 64-bit floating point numbers (e.g., `3.14`, `-0.01`)
- **String**: Double-quoted text (e.g., `"Hello, World!"`)
- **Boolean**: `true` or `false`
- **Null**: `null`

## 2. Variables

Variables are declared using the `var` keyword.

```javascript
var x = 10;
var name = "Phasor";
x = 20; // Assignment
```

**OpCode Mapping**:

- Declaration: `STORE_VAR` (stack-based) or `STORE_VAR_R` (register-based)
- Usage: `LOAD_VAR` (stack-based) or `LOAD_VAR_R` (register-based)

Note: The VM supports both stack-based and register-based variants. The compiler may emit either style.

## 3. Control Flow

### If-Else

```javascript
if (x > 10) {
    print "Greater than 10";
} else if (x > 0) {
    print "Positive";
} else {
    print "Less or equal to zero";
}
```

**OpCode Mapping**: `JUMP_IF_FALSE`, `JUMP`

### While Loop

```javascript
var i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
```

**OpCode Mapping**: `JUMP_IF_FALSE`, `JUMP_BACK`

### For Loop

```javascript
for (var i = 0; i < 5; i = i + 1) {
    print i;
}
```

### Break and Continue

```javascript
while (true) {
    if (condition) break;
    if (skip) continue;
}
```

### Switch/Case Statements

```javascript
var status = 200;
switch (status) {
    case 200:
        puts("OK");
    case 404:
        puts("Not Found");
    case 500:
        puts("Server Error");
    default:
        puts("Unknown");
}
```

Each case must be a constant value. Execution falls through to the next case unless a break is used. The default case is optional.

**OpCode Mapping**: `EQUAL`, `JUMP_IF_FALSE`, `JUMP`

## 4. Functions

Functions are declared with `fn`.

```javascript
fn add(a: int, b: int) -> int {
    return a + b;
}

var result = add(5, 10);
```

*Note: Type annotations (`: int`) are for documentation and are not enforced at runtime.*

**OpCode Mapping**:

- Call: `CALL` (user-defined) or `CALL_NATIVE` (native functions)
- Return: `RETURN`

## 5. Structs

Structs allow grouping related data with static field access at compile time.

```javascript
struct Point {
    x: int,
    y: int
}

var p = Point { x: 10, y: 20 };
var xVal = p.x;
p.x = 15;
```

**OpCode Mapping**:

- Instance creation: `NEW_STRUCT_INSTANCE_STATIC`
- Field access: `GET_FIELD_STATIC` / `SET_FIELD_STATIC`
- Dynamic field access: `GET_FIELD` / `SET_FIELD` (for runtime field names)

## 6. Built-in Functions

Phasor includes a comprehensive standard library. Functions must be registered via include statements. For full reference, see `guide_stdlib.md`.

### Common Functions

```javascript
include_stdio();      // I/O functions
include_stdstr();     // String functions
include_stdmath();    // Math functions
include_stdfile();    // File operations
include_stdsys();     // System functions
include_stdtype();    // Type conversion
include_stdregex();   // Regular expressions

// Output
puts("Hello with newline");      // Print with newline
prints("No newline");           // Print without newline
putf("Value: %d", 42);          // Formatted print

// Input
var line = gets();              // Read line from stdin

// String functions
var length = len("hello");      // Returns 5
var sub = substr("hello", 1, 3);// Returns "ell"
var upper = to_upper("hi");     // Returns "HI"
var lower = to_lower("HI");     // Returns "hi"

// Math functions
var root = math_sqrt(16);       // Returns 4.0
var pow = math_pow(2, 10);      // Returns 1024
var abs = math_abs(-5);         // Returns 5

// Type conversion
var num = to_int("42");         // Returns 42
var str = to_string(123);       // Returns "123"

// System
var ms = time();                // Current time in ms
sleep(1000);                    // Sleep 1 second
var os = sys_os();              // "NT", "Linux", etc.

// File I/O
var content = fread("file.txt");
fwrite("out.txt", "data");
var exists = fexists("test.txt");
```

**OpCode Mapping**:

- `print`: `PRINT` / `PRINT_R` opcodes
- `gets`: `READLINE` / `READLINE_R` opcodes
- `sys_exec`: `SYSTEM` / `SYSTEM_R` opcodes
- Others: `CALL_NATIVE` (native functions via `VM::registerNativeFunction`)

## 7. Unsafe Blocks

Phasor supports "unsafe" blocks for future low-level operations (currently reserved).

```javascript
unsafe {
    // Low-level code
}
```

## 8. Comments

- Single line: `// Comment`

## 9. Operators

### Arithmetic

- `+`, `-`, `*`, `/`, `%` (modulo)

### Comparison

- `==`, `!=`, `<`, `>`, `<=`, `>=`

### Logical

- `&&` (and), `||` (or), `!` (not)

### Unary

- `-` (negate), `!` (not)

### Postfix

- `++` (increment), `--` (decrement) - returns old value before modification

## 10. Example Program

```javascript
include_stdio();
include_stdstr();

puts("Welcome to Phasor!");
prints("Enter your name: ");
var name = gets();

if (len(name) > 0) {
    putf("Hello, %s!\n", name);
} else {
    puts("Hello, Anonymous!");
}

fn factorial(n: int) -> int {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

var result = factorial(5);
putf("5! = %d\n", result);
```
