# Phasor ![Phasor Logo (Hand-drawn sinewave)](https://phasor.pages.dev/assets/logo.webp) Language

A dynamically typed, compiled programming language with a hybrid stack/register-based bytecode virtual machine, *designed* for performance and flexibility.

Phasor is still in beta, as I wish for a **smooth, stable experience** for the final language. The existing implementation needs a ton of work.

You can check out the [website](https://phasor.pages.dev/) as well.

## Language Features

- **Dynamic typing** with integers, floats (IEEE 754, double-percision), strings, booleans, and null. ```var x = 21; // int```
- **Functions** with forced type annotations ```fn func(input: string) -> void { ... }```
- **Structs** with C style static field access, mostly untested. ```struct.member = 14;```
- **Arrays** are being tested with C syntax ```var arrayName[arraySize];```
- **C-style Control flow**: if/else, while, for, switch/case, break/continue
- **Comprehensive standard library**
- **Windows API Bindings**

## Upcoming

- C based standard library replacing remaining C++ parts (**Started**)
- **POSIX API Bindings**

---

## Q/A

> **Q** - Are you just ripping off the best from every language?
>
> **A** - No, some inspiration came from a few places, TS, C99, Zig, Rust, etc 
>
> **Q** - What is this? Why would I even need this?
>
> **A** - Not sure.
>
> **Q** - Is this better than 'Java' or '.NET'?
>
> **A** - I have not tested Phasor against other languages, runtimes, VMs, etc.
>
> **Q** - How is it fast without JIT?
>
> **A** - Easy! Just pray the entire core end up into L1/L2, then just let the CPU handle it! (I hate that this works)

## Quick Start

```bash
# Open phasor shell (optional)
./shell

# Compile and run a program
phasorjit input.phs

# Interactive REPL
phasorrepl

# Compile to bytecode
phasorcompiler input.phs (-o, --output output.phsb)

# Compile to Phasor VM IR
phasorcompiler -i, --ir input.phs (-o, --output output.phir)

# Run bytecode
phasorvm output.phsb

# Compile to Native
cp src/App/Runtime/NativeRuntime_[static,dynamic]_main.cpp main.cpp

phasornative -c, --compiler clang++ -l, --linker lld -s, --source main.cpp input.phs -o, --output output
````

### Example Program

```javascript
include_stdio();
include_stdtype();

fn factorial(n: int) -> int {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

puts("Enter a number:");
var input = gets();
var num = to_int(input);
var result = factorial(num);
putf("%d! = %d\n", num, result);
```

---

## Documentation

- **[Language Guide](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_phasor_language.md&name=Language%20Guide)** - Complete syntax and language features
- **[Standard Library Guide](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_stdlib.md&name=Standard%20Library%20Guide)** - Comprehensive function reference guide
- **[Standard Library Specifications](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fstd%2F2.0.0%2Flibrary.md)** - Standard Library Function Standard Specification 2.x.x
- **[VM Internals](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_vm_internals.md&name=VM%20Internals)** - Virtual machine architecture details
- **[Adding Opcodes](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_adding_opcodes.md&name=Adding%20Opcodes)** - Contributor guide for VM extensions
- **[Doxygen](https://phasor-docs.pages.dev)** - Documentation and call graphs generated from source code
- **Manuals/Man Pages:** `https://phasor-docs.pages.dev/man?f=[filename]`, e.g. `/man?f=Phasor.3`, `/man?f=phasorvm.1`, `/man?f=PHIR.5`, etc.

## Contributing

1. Read the [VM Internals](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_vm_internals.md&name=VM%20Internals) and [Adding Opcodes](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_adding_opcodes.md&name=Adding%20Opcodes) guides
2. Follow the existing code style (see `.clang-format`)
3. Add tests for new features
4. Update documentation as needed

## Applications

- **REPL** ([`phasorrepl`](https://phasor-docs.pages.dev/man?f=phasorrepl.1)) - Interactive interpreter
- **Bytecode Compiler** ([`phasorcompiler`](https://phasor-docs.pages.dev/man?f=phasorcompiler.1)) - Script to bytecode compiler
- **Native Compiler** ([`phasornative`](https://phasor-docs.pages.dev/man?f=phasornative.1)) - Script to C++ transpiler
- **VM Runtime** ([`phasorvm`](https://phasor-docs.pages.dev/man?f=phasorvm.1)) - Bytecode execution engine
- **JIT Runtime** ([`phasorjit`](https://phasor-docs.pages.dev/man?f=phasorjit.1)) - Direct script execution
- **Shell** (`shell`) - Phasor-based command shell **PREVIEW**
- **Core Utils** (`cat-phs`, `cp-phs`, `echo-phs`, `ls-phs`, `mv-phs`, `rm-phs`, `touch-phs`) - Unix-like utilities **PREVIEW**

## Standard Library Modules

| Module      | Include Statement    | Functions                                                             |
| ----------- | -------------------- | --------------------------------------------------------------------- |
| **I/O**     | `include_stdio()`    | `puts`, `puts_error` `printf`, `gets`, `putf`, `msgbox`, `msgbox_err` |
| **Math**    | `include_stdmath()`  | `math_sqrt`, `math_pow`, `math_sin`, `math_cos`, etc.                 |
| **Strings** | `include_stdstr()`   | `len`, `substr`, `concat`, `to_upper`, `to_lower`                     |
| **Files**   | `include_stdfile()`  | `fread`, `fwrite`, `fexists`, `fcopy`, `fmove`                        |
| **System**  | `include_stdsys()`   | `time`, `sleep`, `sys_os`, `sys_exec`, `clear`                        |
| **Types**   | `include_stdtype()`  | `to_int`, `to_float`, `to_string`, `to_bool`                          |
| **Regex**   | `include_stdregex()` | `regex_match`, `regex_search`, `regex_replace`                        |

---

## Building

<sub>Using Arch BTW? Grab the PKGBUILD</sub>

### Prerequisites

- CMake 3.10+
- C++17 compiler (MSVC, GCC, Clang)

### Build Steps

```bash
cmake -S . -B build
cmake --build build --config Release (-DASSEMBLY=[ON,OFF] (Only supported on Windows-AMD64))
cmake --install build --prefix install
````

### Generation/Build Options

- `ASSEMBLY=ON/OFF` - Enable/disable native assembly optimizations (default: OFF)

## Output

Binaries are available in the `install` directory:

- `bin/` - All executables and utilities
- `lib/` - Runtime libraries (static and shared)

---

**Phasor** - Fast, flexible programming/scripting with *near* native VM performance.

This project is licensed under the following agreements, which may be updated at any time, with or without notice, upon a new non-patch release of the language.

- Phasor Language / VM / Toolchain | [Phasor Restrictive License 1.3](https://phasor.pages.dev/document.html?file=content%2Flegal%2Fphasorprl.txt&name=Phasor%20Restrictive%20License%201.3)
- Phasor Standard Library | [Phasor Open License 1.0](https://phasor.pages.dev/document?file=content%2Flegal%2Fphasorpol.txt&name=Phasor%20Open%20License%201.0)
- Phasor Shell | [MIT License](https://opensource.org/license/mit)
- Phasor coreutils implementation | [GNU General Public License 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)
- Phasor binary releases | [Phasor Binary EULA](https://phasor.pages.dev/document?file=content%2Flegal%2Fphasorbineula.txt&name=Phasor%20Binary%20EULA)

Mentions of 'coreutils', the Free Software Foundation, Inc., 'Java™', Oracle® Corporation, '.NET™', Microsoft® Corporation, Google® LLC, or other third-party companies, products, or trademarks do not imply any affiliation, endorsement, or sponsorship by those third parties, or thier affiliates, unless explicitly stated otherwise.

