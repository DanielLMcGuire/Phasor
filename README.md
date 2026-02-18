# Phasor <img src="https://phasor.pages.dev/assets/logo.svg" alt="Phasor Logo (Hand-drawn sinewave)" width="320" height="160"> Language

A statically typed, compiled programming language with a hybrid stack/register-based bytecode virtual machine, *designed* for flexibility.

Phasor is still in beta, as I wish for a **smooth, stable experience** for the final language. The existing implementation still needs a vast amount of work. Although it's perfectly stable now, at least enough for me to prototype with.

You can check out the [website](https://phasor.pages.dev/) as well.

---

## Language Features

- **Dynamic typing** with integers, floats (IEEE 754, double-percision), strings, booleans, and null. ```var x = 21; // int```
- **Functions** with forced type annotations ```fn func(input: string) -> void { ... }```
- **C-style Control flow**: if/else, while, for, switch/case, break/continue
- **Comprehensive standard library**
- **Windows and POSIX API Bindings**

## Upcoming 
> [!NOTE]
>
> Some may *appear* to work before actual implementation

- **Structs** with C style static field access ```struct.member = 14;```
- **Arrays** with C syntax ```var arrayName[arraySize];``` 

---

## Q/A

> **Q** - Are you just ripping off the best from every language?
>
> **A** - No, the idea started before I knew much about other languages, some inspiration however did come from the majority of the programming languages I use. <sub>(Like C++, Python, TS, C to name a few)</sub>
>
> **Q** - What is this? Why would I even need this?
>
> **A** - You probably don't.
>
> **Q** - Is this better than 'Java' or '.NET'?
>
> **A** - Phasor doesn't beat anything without PGO (which even this is determinisic)
>
> **Q** - How is it somewhat fast without JIT?
>
> **A** - Easy! Just pray the entire core end up into L1/L2, then just let the CPU handle it! (Yes, this actually works, no joke)

## Quick Start

```bash
# Compile and run a program
phasorjit input.phs # or use 'phasor input.phs'

# Interactive REPL 
phasorrepl # or use 'phasor'

# Compile to bytecode
phasorcompiler input.phs (-o, --output output.phsb)

# Compile to Phasor VM IR
phasorcompiler -i, --ir input.phs (-o, --output output.phir)

# Run bytecode
phasorvm output.phsb # or use 'phasor output.phsb'

# Compile to Native
cp src/App/Runtime/NativeRuntime_[static,dynamic]_main.cpp main.cpp

# Run a script raw
cat input.phs | phasor

phasornative -c, --compiler clang++ -l, --linker lld -s, --source main.cpp input.phs -o, --output output
```

### Example Program

```javascript
include_stdio();
include_stdtype();

puts("Enter a number:");
var input = gets();
var num1 = to_int(input);
var num2 = 25;
putf("%d + %d = %d\n", num, num2, num1 + num2);
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

- **phasor** - Combines the JIT Runtime, VM Runtime, and REPL, as well as adding pipe-in for raw scripts, and supports shabangs
- **REPL** ([`phasorrepl`](https://phasor-docs.pages.dev/man?f=phasorrepl.1)) - Interactive interpreter
- **Bytecode Compiler** ([`phasorcompiler`](https://phasor-docs.pages.dev/man?f=phasorcompiler.1)) - Script to bytecode compiler
- **Native Compiler** ([`phasornative`](https://phasor-docs.pages.dev/man?f=phasornative.1)) - Script to C++ transpiler
- **VM Runtime** ([`phasorvm`](https://phasor-docs.pages.dev/man?f=phasorvm.1)) - Bytecode executor
- **JIT Runtime** ([`phasorjit`](https://phasor-docs.pages.dev/man?f=phasorjit.1)) - Direct script executor
- **Shell** (`shell`) - Phasor-based command shell **PREVIEW**
- **Core Utils** (`cat-phs`, `cp-phs`, `echo-phs`, `ls-phs`, `mv-phs`, `rm-phs`, `touch-phs`) - Unix-like utilities **PREVIEW**

---

## Overview

This repo contains:

- Frontend:
  - [Phasor Language](https://github.com/DanielLMcGuire/Phasor/blob/master/docs/man/man5/PHS.5) (Specification)
  - [Phasor Runtime](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Runtime) / [VM](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Runtime/VM) ([ISA Specs](https://github.com/DanielLMcGuire/Phasor/blob/master/docs/man/man7/phasor-isa.7), C/C++/Assembly)
  - [Phasor Standard Library](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Runtime/Stdlib) ([Specifications](https://github.com/DanielLMcGuire/Phasor/tree/master/docs/man/man3), C/C++)
 
- Backend:
  - [Phasor Compiler Infrastructure](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Backend) (C++)
  - [FFI API](https://github.com/DanielLMcGuire/Phasor/blob/master/include/PhasorFFI.h) (C)
  - [Runtime API](https://github.com/DanielLMcGuire/Phasor/blob/master/include/PhasorRT.h) (C)

- Extensions:
  - [Phasor](https://github.com/DanielLMcGuire/Phasor/blob/master/src/Extensions/Phasor.tmLanguage) & [Phasor IR](https://github.com/DanielLMcGuire/Phasor/blob/master/src/Extensions/phasor-ir.tmLanguage) TextMate Grammar
  -  [Phasor Visual Studio Code Extension](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Extensions/vscode) (Typescript)

---

## Building

<sub>Using Arch BTW? Grab the PKGBUILD</sub>

### Prerequisites

- CMake 3.10+
- C++20 compiler (MSVC, GCC, Clang)
- Ninja

### Build Steps

```bash
# --prefix [windows,macos,linux]-[32,64,arm64]-[rel,dbg]
cmake -S . -B build -G Ninja --prefix windows-64-rel
cmake --build build
cmake --install build --prefix install
````

### Generation/Build Options

- `ASSEMBLY=ON/OFF` - Enable/disable native assembly optimizations (default: OFF)

## Output

Binaries are available in the `install` directory:

- `bin/` - All executables and utilities
- `lib/` - Runtime libraries (static and shared)

Plugins (Like win32 api, posix) are available in different locations based on your OS:

- Unix - `opt/Phasor/plugins/`
- macOS - `library/Application Support/org.Phasor.Phasor/plugins`
- Windows - `bin/plugins` 

---

**Phasor** - Fast, flexible programming/scripting with *near* native VM performance.

- Phasor Language / ISA / VM / Toolchain / Standard Library | [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt)
- Phasor Shell | [MIT License](https://opensource.org/license/mit)
- Phasor coreutils implementation | [GNU General Public License 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)

Mentions of 'coreutils', the Free Software Foundation, Inc., 'Java™', Oracle® Corporation, '.NET™', Microsoft® Corporation, Google® LLC, or other third-party companies, products, or trademarks do not imply any affiliation, endorsement, or sponsorship by those third parties, or thier affiliates, unless explicitly stated otherwise.

Phasor and the "sinewave Phasor" logo are trademarks of Daniel McGuire.
