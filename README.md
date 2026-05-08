# Phasor <kbd><img src="https://phasor.pages.dev/assets/logo.svg" width="250" height="130"></kbd> Language

[![Release](https://img.shields.io/github/v/release/DanielLMcGuire/Phasor.svg)](https://phasor.pages.dev/downloads?version=latest)
[![AUR Version](https://img.shields.io/aur/version/phasor.svg)](https://aur.archlinux.org/packages/phasor)
![GitHub branch check runs](https://img.shields.io/github/check-runs/DanielLMcGuire/Phasor/master.svg)
![GitHub commit activity](https://img.shields.io/github/commit-activity/w/DanielLMcGuire/Phasor.svg?label=commits)

A dynamically typed, compiled programming language with a fast bytecode virtual machine. Parameter and return types are static for user defined (non FFI / non STDLIB) functions.

Phasor *does not* have a traditional garbage collector, the entire toolchain makes use of my unified safe type system, which provides C++ RAII support to the runtime, stdlib, but user memory management is manual for now.

See [Language Features](#language-features) for more info on memory management.

See [Building](#building) for info on building from source.

Phasor is stable, but still in beta, as I wish for a **smooth, stable experience** for the final language. The existing implementation still needs ***some* work**. The ABI is not stable, but conforms to semver most of the time (thus why this is `3.X.X` and still in beta.)

You can check out the [website](https://phasor.pages.dev/) as well.

[Download Latest Stable](https://github.com/DanielLMcGuire/Phasor/releases/latest)

[Download Latest Nightly](https://github.com/DanielLMcGuire/Phasor/actions/workflows/nightly.yml?query=is%3Asuccess+branch%3Amaster)

```bash
# Run via nix
nix run github:DanielLMcGuire/Phasor -- -- <options>
# Run directly (requires install)
phasor <options>
```

## Language Features

- **Dynamic typing** with integers, floats (IEEE 754, double-percision), strings, booleans, and null. ```var x = 21; // int```
- **Type annotations** (only in function declarations) ```fn func(input: string) -> void { ... }```
- **Control flow**: if/else, while, for, switch/case, break/continue
- **Standard library** ```using(featureName: string)```
- **Plugin/FFI API** [PhasorFFI.h](include/PhasorFFI.h)
- **[Runtime API](https://phasor-docs.pages.dev/man?f=phasorrt.3)** [PhasorRT.h](include/PhasorRT.h)
- **[Rust runtime bindings](#overview)** (capi wrapper) (already in master!)
- **[Zig runtime bindings](#overview)** (capi wrapper) (already in master!)
- **Minimal Windows and POSIX API Bindings**
- Supports *most* [**C format specifiers**](https://www.geeksforgeeks.org/c/format-specifiers-in-c/)
- **stdmem** stdlib module with free() ```using("stdmem"); free("variableName");``` (same as `something = null;`)
- **AppleScript bindings** for phasor

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
>
> The existance of a keyword/token type does not imply there is planned support for said feature

- **Structs** with C style static field access ```struct.member = 14;```
- **Arrays** with C syntax ```var arrayName[arraySize];```
- **ActiveX Scripting Engine (COM)** (partial implementation already in 3.3.0)

---

## Quick Start

```bash
# Compile and run a program
$ phasor input.phs

# Repl
$ phasor

# Compile to bytecode
$ phasorcompiler input.phs (-o, --output output.phsb)

# Run bytecode
$ phasorvm output.phsb

# Run a script raw
$ echo "print \"Hello World\!\\n\"" | phasor
Hello World!
$
```

### Example Program

```javascript
using("stdio", "stdtype", "stdmem");

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
> Online docs are always up to date with master (as much as I can at least), offline (installed) docs are always usually up to date with that version.
>
> Feel free to make an issue if something is not right.

- **[Language Guide](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_phasor_language.md&name=Language%20Guide)** - Complete syntax and language features
- **[VM Internals](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_vm_internals.md&name=VM%20Internals)** - Virtual machine architecture details
- **[Adding Opcodes](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_adding_opcodes.md&name=Adding%20Opcodes)** - Contributor guide for VM extensions
- **[Doxygen](https://phasor-docs.pages.dev)** - Documentation and call graphs generated from source code
- **Manuals/Man Pages:** `https://phasor-docs.pages.dev/man?f=[filename]`, e.g. `/man?f=phasor.1` See [here](docs/man/) for all man pages.

## Contributing

1. Read the [VM Internals](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_vm_internals.md&name=VM%20Internals) and [Adding Opcodes](https://phasor.pages.dev/document?file=https%3A%2F%2Fphasor-docs.pages.dev%2Fcontent%2Fguide_adding_opcodes.md&name=Adding%20Opcodes) guides
2. Follow the existing code style (see `.clang-format`)
3. Add tests for new features
4. Update documentation as needed

## Applications

- **Phasor**
  - **[phasor](https://phasor-docs.pages.dev/man?f=phasor.1)** - Combines the Scripting Runtime, VM Runtime, and REPL, adds pipe support, and supports shabangs
  - **[phasor-lsp](https://phasor-docs.pages.dev/man?f=phasorlsp.1)** - JSON-RPC 2.0 LSP Protocol Handler for the Phasor Language
  - **Bytecode Compiler** ([`phasorcompiler`](https://phasor-docs.pages.dev/man?f=phasorcompiler.1)) - Script to bytecode compiler
  - **Native Compiler** ([`phasornative`](https://phasor-docs.pages.dev/man?f=phasornative.1)) - Script to C++ transpiler
  - **VM Runtime** ([`phasorvm`](https://phasor-docs.pages.dev/man?f=phasorvm.1)) - Bytecode executor
  - **Disassembler** ([`phasordecomp`](https://phasor-docs.pages.dev/man?f=phasordecomp.1))
  - **Assembler** ([`phasorasm`](https://phasor-docs.pages.dev/man?f=phasorasm.1))

- **Pulsar**
  - **[pulsar](https://phasor-docs.pages.dev/man?f=pulsar.1)** - Scripting runtime and REPL
  - **Bytecode Compiler** ([`pulsarcompiler`](https://phasor-docs.pages.dev/man?f=pulsarcompiler.1)) - Script to bytecode compiler
  - Uses the Phasor VM Runtime for bytecode ([`phasorvm`](https://phasor-docs.pages.dev/man?f=phasorvm.1))

---

## Overview

This repo contains:

- Frontend:
  - [Phasor Language](src/Language/Phasor) ([Specifications](https://phasor-docs.pages.dev/man?f=PHS.5), C++)
  - [Pulsar Language](src/Language/Pulsar) (C++)
  - [Phasor Runtime](src/Runtime) / [VM](src/Runtime/VM) ([ISA Specs](https://phasor-docs.pages.dev/man?f=phasor-isa.7), C/C++/Assembly)
  - [Phasor Standard Library](src/Runtime/Stdlib) ([Man pages](docs/man/man3), C/C++)
- Backend:
  - [Phasor Compiler Infrastructure](src/Codegen) (C++)
  - [FFI API](include/PhasorFFI.h) (C)
  - [Runtime API](include/PhasorRT.h) (C)
- Extensions:
  - [Phasor](src/Extensions/Phasor.tmLanguage) & [Phasor IR](src/Extensions/phasor-ir.tmLanguage) TextMate Grammar
  - [Phasor Visual Studio Code Extension](src/Extensions/vscode) (Typescript)
  - [Python Module](https://phasor-docs.pages.dev/man?f=phasor-py.3) `src/Extensions/py/phasor`

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

  - [PowerShell Module (windows only, phasorrt.dll bindings)](src/Extensions/powershell) `src/Extensions/powershell/Phasor`
  
    ```powershell
    Import-Module .\src\Extensions\powershell\Phasor\Phasor.psd1
    Get-Help Phasor
    # Get-Help <cmdlet>

    # state is always optional (-State is optional)
    # anything not using state will create thier own
    Start-PhasorScript -ScriptPath .\hello.phs
    Start-PulsarScript -ScriptPath .\hello.pul

    $vm = New-PhasorState
    # stdlib is already registered if not using your own state
    Register-PhasorStdlib -State $vm 

    Start-PhasorEval -State $vm -Script 'var x = 42; var y = 53;'
    # x and y still in scope
    Start-PhasorEval -State $vm -Script 'print(x + y);' 
    
    # all states are cleared at powershell shutdown, but to clear them early:
    Remove-PhasorState $vm

    $bytecode = Build-PhasorScript -Script 'print("hi\n");'
    # can also optionally accept state
    Invoke-PhasorBytecode -Bytecode $bytecode
    ```

  - [phasor-web REST API](src/Extensions/web) `src/Extensions/web` (Typescript, Node 22)

    ```bash
    $ curl -d 'using("stdio"); puts("Hi!");' -H "x-api-key: API_KEY" http://0.0.0.0:62811/run
    {stdout: "Hi!\n", stderr: "", exitCode = 0}
    ```

- Bindings:
  - [phasorrt-rs Rust Bindings (runtime)](https://phasor-docs.pages.dev/man?f=phasorrt-rs.3) `src/Rust` (Rust)

    ```rust
    // Create a new VM and load the standard library
    let mut vm = PhasorVM::new()?;
    vm.init_stdlib()?;

    // run script
    vm.evaluate_phs("print(\"Hello, World!\\n\");", "hello", None, false)?;

    // state is kept 
    vm.evaluate_phs("var x = 42; var y = 53;", "vars", None, false)?;
    vm.evaluate_phs("print(x + y);", "vars", None, false)?;   // prints 95

    // reset vars
    vm.reset(false, true)?;

    // compile/run bytecode
    let bytecode = compile_phs("print(\"hi\\n\");", "hi", None)?;
    vm.exec(&bytecode, "hi", &[])?;

    // VM is automatically freed when it goes out of scope (Drop)
    ```

  - [phasorrt-zig Zig bindings (runtime)](https://phasor-docs.pages.dev/man?f=phasorrt-zig.3) `src/Zig` (Zig)

    ```zig
    const phasor = @import("phasor.zig");
    
    // create vm (auto inits stdlib)
    const vm = try phasor.State.create();
    defer vm.deinit();
    
    // run a script
    _ = try vm.evaluatePHS("print(\"Hello, World!\\n\");", "hello", ".", false);
    
    // state is kept between calls
    _ = try vm.evaluatePHS("var x = 42; var y = 53;", "vars", ".", false);
    _ = try vm.evaluatePHS("print(x + y);", "vars", ".", false); // prints 95
    
    // Reset vars only, keep function definitions
    try vm.reset(false, true);
    
    // compile to bytecode, then execute
    const bytecode = try phasor.compilePHS(allocator, "print(\"hi\\n\");", "hi", ".");
    defer allocator.free(bytecode);
    _ = try vm.exec(bytecode, "hi", &.{});
    
    // VM is freed when deinit() is called (or via defer)
    ```

---

## Building

<sub>Using Arch BTW? Just run `makepkg -si`, or get `phasor-git` on the AUR</sub>

> [!NOTE]
>
> If you have forked and do not plan to contribute back, you should modify `cmake/Version.cmake` to match your forks URL!

### Prerequisites

- Required:
  - [Ninja](https://github.com/ninja-build/ninja/releases)
  - [CMake 3.21+](https://cmake.org/download/)
  - CC (C 23 Compliant) compiler and CXX (C++ 23 Compliant) compiler supporting GCC/Clang extensions or Windows specific extensions
    - [MSVC VS22 17.2 or later](https://visualstudio.microsoft.com/downloads/?q=build+tools)
    - [GCC 14 or later](https://gcc.gnu.org/install/)
    - [Clang 17 or later](https://releases.llvm.org/)
- Optional:
  - [Rust Toolchain (latest stable)](https://rust-lang.org/tools/install/) for the `phasorrt-rs` lib
  - [CPython 3.9 or later](https://www.python.org/downloads/) (or another Python compliant interpreter) for the Python API
  - [PowerShell latest](https://github.com/PowerShell/PowerShell/releases/) for the PowerShell module
  - [Node.js 24 or later](https://nodejs.org/en/download) for the web server, and for building the Visual Studio Code extension
  - [Docker](https://docs.docker.com/get-started/get-docker/) for the man page to pdf conversions (required only for phasor-help on win32)

### Build Steps

See [Building Phasor](https://github.com/DanielLMcGuire/Phasor/wiki/Building-Phasor).

See [Building Phasor Extensions](https://github.com/DanielLMcGuire/Phasor/wiki/Building-Phasor-Extensions) for info on building Extensions/bindings.

### FFI Plugin locations

Available in different locations based on your OS:

- Unix - `/usr/lib/phasor/plugins/`
- macOS - `/Library/Application Support/org.Phasor.Phasor/plugins/`
- Windows - `C:\Program Files\Phasor Programming Language\bin\plugins\`

---

[GitHub](https://github.com/DanielLMcGuire/Phasor)
[GitLab Mirror](https://gitlab.com/DanielLMcGuire/Phasor)
[Codeberg Mirror](https://codeberg.org/DanielLMcGuire/Phasor)

Phasor will be moving to Codeberg (or something similar) in the coming months, CI will stay on github as its entire free, but PRs and issues will eventually be disabled.

Mentions of the Free Software Foundation, Inc., 'Java™', Oracle® Corporation, '.NET™', Microsoft® Corporation, Google® LLC, or other third-party companies, products, or trademarks do not imply any affiliation, endorsement, or sponsorship by those third parties, or thier affiliates, unless explicitly stated otherwise.

Phasor Toolchain is licensed for use under the Apache 2.0 License.

Phasor Runtime (`phasorrt.dll`, `libphasorrt.so`, `libphasorrt.dylib`, `out/lib/**/*`, `out/include/**/*`, etc) is licensed for use under the Apache 2.0 with LLVM-Exceptions License.

Phasor and the "sinewave Phasor" logo are trademarks of Daniel McGuire.
