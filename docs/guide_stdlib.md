# Phasor Standard Library Reference

The Phasor Standard Library (StdLib) provides essential functionality to Phasor programs. It is implemented in C++ and interacts directly with the VM.

## Library Registration

Standard library functions are organized into modules that must be explicitly registered in Phasor code via include statements. Each include statement registers a set of native functions:

```javascript
include_stdio();      // I/O functions
include_stdmath();    // Math functions
include_stdstr();     // String functions
include_stdfile();    // File operations
include_stdsys();     // System functions
include_stdtype();    // Type conversion
include_stdregex();   // Regular expressions
```

## Available Functions

### Math Functions

Registered via `registerMathFunctions()` in `src/Runtime/Stdlib/math.cpp`.

| Function | Arguments | Description |
|----------|-----------|-------------|
| `math_sqrt` | `(x)` | Square root of x |
| `math_pow` | `(base, exp)` | base raised to power exp |
| `math_abs` | `(x)` | Absolute value |
| `math_floor` | `(x)` | Floor of float value |
| `math_ceil` | `(x)` | Ceiling of float value |
| `math_round` | `(x)` | Round to nearest integer |
| `math_min` | `(a, b)` | Minimum of two values |
| `math_max` | `(a, b)` | Maximum of two values |
| `math_log` | `(x)` | Natural logarithm |
| `math_exp` | `(x)` | Exponential (e^x) |
| `math_sin` | `(x)` | Sine (radians) |
| `math_cos` | `(x)` | Cosine (radians) |
| `math_tan` | `(x)` | Tangent (radians) |

### String Functions

Registered via `registerStringFunctions()` in `src/Runtime/Stdlib/string.cpp`. Must be registered with `include_stdstr()` in Phasor code.

| Function | Arguments | Description |
|----------|-----------|-------------|
| `len` | `(str)` | String length |
| `substr` | `(str, start, [len])` | Substring extraction |
| `concat` | `(a, b)` | Concatenate strings |
| `to_upper` | `(str)` | Convert to uppercase |
| `to_lower` | `(str)` | Convert to lowercase |
| `char_at` | `(str, index)` | Get character at index |
| `starts_with` | `(str, prefix)` | Check if string starts with prefix |
| `ends_with` | `(str, suffix)` | Check if string ends with suffix |

### I/O Functions

Registered via `registerIOFunctions()` in `src/Runtime/Stdlib/io.cpp`. Must be registered with `include_stdio()` in Phasor code.

| Function | Arguments | Description |
|----------|-----------|-------------|
| `prints` | `(str)` | Print string without newline |
| `printf` | `(fmt, ...)` | Print formatted string |
| `puts` | `(str)` | Print string with newline |
| `putf` | `(fmt, ...)` | Print formatted with newline |
| `gets` | `()` | Read line from stdin |
| `puts_error` | `(str)` | Print to stderr with newline |
| `putf_error` | `(fmt, ...)` | Print formatted to stderr |
| `msgbox` | `(title, msg)` | Show message box (Windows only) |
| `c_fmt` | `(fmt, ...)` | C-style string format |

**Format Specifiers** (for `printf`, `putf`, `c_fmt`):

- `%s` - String
- `%d` - Integer
- `%f` - Float
- `%%` - Literal percent sign

### File Functions

Registered via `registerFileFunctions()` in `src/Runtime/Stdlib/file.cpp`. Must be registered with `include_stdfile()` in Phasor code.

| Function | Arguments | Description |
|----------|-----------|-------------|
| `fread` | `(path)` | Read entire file as string |
| `fwrite` | `(path, content)` | Write string to file |
| `fexists` | `(path)` | Check if file exists (bool) |
| `fread_line` | `(path)` | Read a line from file |
| `fwrite_line` | `(path, line)` | Write a line to file |
| `fappend` | `(path, content)` | Append to file |
| `fdelete` | `(path)` | Delete file |
| `frename` | `(old, new)` | Rename file |
| `fcopy` | `(src, dst)` | Copy file |
| `fmove` | `(src, dst)` | Move file |
| `fcwd` | `([path])` | Get/set current working directory |
| `fdir_create` | `(path)` | Create directory |
| `fdir_remove` | `(path)` | Remove directory |
| `fdir_read` | `(path)` | List directory contents |
| `fstat` | `(path)` | Get file statistics |

### System Functions

Registered via `registerSysFunctions()` in `src/Runtime/Stdlib/system.cpp`. Must be registered with `include_stdsys()` in Phasor code.

| Function | Arguments | Description |
|----------|-----------|-------------|
| `time` | `()` | Current time in milliseconds |
| `timef` | `(format)` | Formatted time string (strftime) |
| `sleep` | `(ms)` | Sleep for milliseconds |
| `clear` | `()` | Clear console screen |
| `sys_os` | `()` | Get OS name ("NT", "Linux", etc.) |
| `sys_env` | `(key)` | Get environment variable |
| `sys_argv` | `(index)` | Get command line argument |
| `sys_argc` | `()` | Get argument count |
| `wait_for_input` | `()` | Wait for any input |
| `sys_exec` | `(cmd)` | Execute shell command |

### Type Conversion Functions

Registered via `registerTypeConvFunctions()` in `src/Runtime/Stdlib/typeconv.cpp`. Must be registered with `include_stdtype()` in Phasor code.

| Function | Arguments | Description |
|----------|-----------|-------------|
| `to_int` | `(val)` | Convert to integer |
| `to_float` | `(val)` | Convert to float |
| `to_string` | `(val)` | Convert to string |
| `to_bool` | `(val)` | Convert to boolean |

### Regex Functions

Registered via `registerRegexFunctions()` in `src/Runtime/Stdlib/regex.cpp`. Must be registered with `include_stdregex()` in Phasor code.

| Function | Arguments | Description |
|----------|-----------|-------------|
| `regex_match` | `(str, pattern)` | Check if regex matches entire string |
| `regex_search` | `(str, pattern)` | Check if regex is found in string |
| `regex_findall` | `(str, pattern)` | Find all regex matches |
| `regex_split` | `(str, pattern)` | Split string by regex matches |
| `regex_replace` | `(str, pattern, replacement)` | Replace regex matches |

---

## Extending the Standard Library

### 1. Function Signature

All native functions must match the `NativeFunction` signature defined in `src/Runtime/VM/VM.hpp`:

```cpp
using NativeFunction = std::function<Value(const std::vector<Value>& args, VM* vm)>;
```

- **args**: Vector of `Value` arguments (left-to-right order).
- **vm**: Pointer to VM instance for state access.
- **Return**: Must return a `Value` object.

### 2. Implementation Example

```cpp
Value my_custom_fn(const std::vector<Value>& args, VM *vm)
{
    checkArgCount(args, 2, "my_fn");  // Validate arg count
    int64_t a = args[0].asInt();
    int64_t b = args[1].asInt();
    return Value(a + b);
}
```

### 3. Registration

Add to the appropriate register function (or create new category file):

```cpp
void StdLib::registerMyFunctions(VM &vm)
{
    vm.registerNativeFunction("my_fn", my_custom_fn);
}
```

### 4. Best Practices

- **Type Checking**: Use `Value::isInt()`, `Value::isString()`, etc.
- **Error Handling**: Throw `std::runtime_error` for runtime errors.
- **Naming**: Use hyphen-style (`my-fn`) or snake_case (`my_fn`).
