# Phasor <img src="https://phasor.pages.dev/assets/logo.svg" alt="Phasor Logo (Hand-drawn sinewave)" width="320" height="160"> Language

[![Release](https://img.shields.io/github/v/release/DanielLMcGuire/Phasor)](https://phasor.pages.dev/downloads?version=latest)
[![AUR Version](https://img.shields.io/aur/version/phasor)](https://aur.archlinux.org/packages/phasor)
[![Visual Studio Marketplace Version](https://img.shields.io/visual-studio-marketplace/v/Phasor.phasor-programming-language?label=VSCODE%20version)](https://marketplace.visualstudio.com/items?itemName=Phasor.phasor-programming-language)
![GitHub branch check runs](https://img.shields.io/github/check-runs/DanielLMcGuire/Phasor/master.svg)

A statically typed, compiled programming language with a fast bytecode virtual machine.

Phasor *does not* have a traditional garbage collector, the entire toolchain makes use of my unified safe type system, which provides RAII support to the runtime.

Phasor is stable, but still in beta, as I wish for a **smooth, stable experience** for the final language. The existing implementation still needs some work.

You can check out the [website](https://phasor.pages.dev/) as well.

[Download Phasor Nightly](https://github.com/DanielLMcGuire/Phasor/actions/workflows/nightly.yml?query=is%3Asuccess+branch%3Amaster)

```bash
$ nix run github:DanielLMcGuire/Phasor
```

## Language Features

- **Dynamic typing** with integers, floats (IEEE 754, double-percision), strings, booleans, and null. ```var x = 21; // int```
- **Type annotations** (only in function declarations) ```fn func(input: string) -> void { ... }```
- **Control flow**: if/else, while, for, switch/case, break/continue
- **Standard library** ```using(featureName: string)```
- **Plugin/FFI API** [PhasorFFI.h](https://github.com/DanielLMcGuire/Phasor/blob/master/include/PhasorFFI.h)
- **Runtime API** [PhasorRT.h](https://github.com/DanielLMcGuire/Phasor/blob/master/include/PhasorRT.h)
- **Minimal Windows and POSIX API Bindings**
- Supports *most* [**C format specifiers**](https://www.geeksforgeeks.org/c/format-specifiers-in-c/)

```javascript
// Print (keyword)
print "Hello World!\n"; // Print to console
// Or via std
using("stdio"); // Import io for puts
puts("Hello World!"); // Print string with newline
```
```javascript
using("stdsys", "stdio"); // Import sys, io
// Variables
var code = 15; // int
var fmt = "Code = %d"; // string
// Formatting
printf("Exiting with code %d", code);
putf("%d", code);
var fmtStr = c_fmt(fmt, code);
// Exit with a code other than 0
shutdown(code); // from stdsys
```

## Upcoming 
> [!IMPORTANT]
>
> Some may *appear* to work before actual implementation

- **Structs** with C style static field access ```struct.member = 14;```
- **Arrays** with C syntax ```var arrayName[arraySize];``` 

---

## Quick Start

```bash
# Compile and run a program
$ phasor input.phs

# Interactive REPL 
$ phasorrepl # or use 'phasor'

# Compile to bytecode
$ phasorcompiler input.phs (-o, --output output.phsb)

# Run bytecode
$ phasorvm output.phsb # or use 'phasor output.phsb'

# Run a script raw
$ echo "print \"Hello World!\\n\"" | phasor
Hello World!
$
```

### Example Program

```javascript
using("stdio", "stdtype");

puts("Enter a number:");
var input = gets();
var num1 = to_int(input);
var num2 = 25;
putf("%d + %d = %d\n", num, num2, num1 + num2);
```

---

## Documentation

> [!NOTE]
>
> Documentation may be partially AI generated, always confirm the feature works first!!
>
> This will change once I have more time for this project


- **[Language Guide](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_phasor_language.md&name=Language%20Guide)** - Complete syntax and language features
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

- **[phasor](https://phasor-docs.pages.dev/man?f=phasor.1)** - Combines the Scripting Runtime, VM Runtime, and REPL, adds pipe support, and supports shabangs
- **[phasor-lsp](https://phasor-docs.pages.dev/man?f=phasorlsp.1)** - JSON-RPC 2.0 LSP Protocol Handler for the Phasor Language
- **REPL** ([`phasorrepl`](https://phasor-docs.pages.dev/man?f=phasorrepl.1)) - Interactive interpreter
- **Bytecode Compiler** ([`phasorcompiler`](https://phasor-docs.pages.dev/man?f=phasorcompiler.1)) - Script to bytecode compiler
- **Native Compiler** ([`phasornative`](https://phasor-docs.pages.dev/man?f=phasornative.1)) - Script to C++ transpiler
- **VM Runtime** ([`phasorvm`](https://phasor-docs.pages.dev/man?f=phasorvm.1)) - Bytecode executor
- **Disassembler** ([`phasordecomp`](https://phasor-docs.pages.dev/man?f=phasordecomp.1))
- **Assembler** ([`phasorasm`](https://phasor-docs.pages.dev/man?f=phasorasm.1))

---

## Overview

This repo contains:

- Frontend:
  - [Phasor Language](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Language/Phasor) ([Specifications](https://phasor-docs.pages.dev/man?f=PHS.5), C++)
  - [Pulsar Language](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Language/Pulsar) (C++)
  - [Phasor Runtime](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Runtime) / [VM](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Runtime/VM) ([ISA Specs](https://phasor-docs.pages.dev/man?f=phasor-isa.7), C/C++/Assembly)
  - [Phasor Standard Library](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Runtime/Stdlib) ([Specifications](https://github.com/DanielLMcGuire/Phasor/tree/master/docs/man/man3), C/C++)
 
- Backend:
  - [Phasor Compiler Infrastructure](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Codegen) (C++)
  - [FFI API](https://github.com/DanielLMcGuire/Phasor/blob/master/include/PhasorFFI.h) (C)
  - [Runtime API](https://github.com/DanielLMcGuire/Phasor/blob/master/include/PhasorRT.h) (C)

- Extensions:
  - [Phasor](https://github.com/DanielLMcGuire/Phasor/blob/master/src/Extensions/Phasor.tmLanguage) & [Phasor IR](https://github.com/DanielLMcGuire/Phasor/blob/master/src/Extensions/phasor-ir.tmLanguage) TextMate Grammar
  -  [Phasor Visual Studio Code Extension](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Extensions/vscode) (Typescript)

  - [Python Module ](https://phasor-docs.pages.dev/man?f=phasor-py.3) `src/Extensions/py`
    ```python
    from phasor import Bytecode, Value, OpCode, Runtime
    help(phasor)

    bc = Bytecode()
    bc.emit(OpCode.PUSH_CONST, bc.add_constant(Value.from_string("Hello, World!\n")))
    bc.emit(OpCode.PRINT)
    bc.emit(OpCode.HALT)
    bc.save("hello_world.phsb")
    Runtime.run(bc) # wraps phasorrt lib
    ```

  - [PowerShell Module (windows only, phasorrt.dll bindings)](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Extensions/powershell) `src/Extensions/powershell`
    ```powershell
    Import-Module .\src\Extensions\powershell\Phasor.psd1
    Get-Help Phasor

    # state is always optional
    Start-PhasorScript -ScriptPath .\hello.phs
    Start-PulsarScript -ScriptPath .\hello.pul

    $vm = New-PhasorState
    Register-PhasorStdlib -State $vm # automatic if not managing manually

    Start-PhasorEval -State $vm -Script 'var x = 42; var y = 53;'
    Start-PhasorEval -State $vm -Script 'print(x + y);'   # x and y still in scope
    Remove-PhasorState $vm # avoid leaking

    $bytecode = Build-PhasorScript -Script 'print("hi\n");'
    Invoke-PhasorBytecode -Bytecode $bytecode
    ```

  - [phasor-web REST API](https://github.com/DanielLMcGuire/Phasor/tree/master/src/Extensions/web) `src/Extensions/web` (Typescript, Node 22)
    ```bash
    $ curl -d 'using("stdio"); puts("Hi!");' -H "x-api-key: API_KEY" http://0.0.0.0:62811/run
    {stdout: "Hi!\n", stderr: "", exitCode = 0}
    ```

---

## Building

<sub>Using Arch BTW? Just run `makepkg -si`</sub>

### Prerequisites

- CMake 3.21+
- CC (23) compiler and CXX (23) compiler (MSVC latest, GCC 14, Clang (LLVM Latest) are officially supported)
- Ninja

### Build Steps

[Moved over to the wiki](https://github.com/DanielLMcGuire/Phasor/wiki/Building-Phasor)

### Plugin locations
(Like win32 api, posix) are available in different locations based on your OS:

- Unix - `/usr/lib/phasor/plugins/`
- macOS - `/library/Application Support/org.Phasor.Phasor/plugins/`
- Windows - `C:\Program Files\Phasor Programming Language\bin\plugins\` 

---

**Phasor** - Fast, flexible programming/scripting with *near* native VM performance.

[GitLab Mirror](https://gitlab.com/DanielLMcGuire/Phasor)

- Phasor Language / ISA / VM / Toolchain / Standard Library | [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt)
- Phasor Shell | [MIT License](https://opensource.org/license/mit)
- Phasor coreutils implementation | [GNU General Public License 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)

To avoid legal complications, Phasor will not be officially available on Linux-based (and many Unix-based) operating systems in the U.S. states of Colorado effective 1-1-2028 and California effective 1-1-2027 due to lack of stable struct/array support, therefore lacking D-Bus / OS specific compatibility without unofficial plugins.

Mentions of 'coreutils', the Free Software Foundation, Inc., 'Java™', Oracle® Corporation, '.NET™', Microsoft® Corporation, Google® LLC, or other third-party companies, products, or trademarks do not imply any affiliation, endorsement, or sponsorship by those third parties, or thier affiliates, unless explicitly stated otherwise.

Phasor and the "sinewave Phasor" logo are trademarks of Daniel McGuire.
