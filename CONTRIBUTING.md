## Contributor Guide

### Pipeline
**Lexer** → **Parser** → **AST** → **Codegen** → **Bytecode/IR**

1. **Language/** - Lexer tokenizes source, Parser builds AST from tokens
2. **AST/** - Node definitions (Expression, Statement, etc.)
3. **Codegen/** - Walks AST, emits instructions, performs constant folding & type inference
   - `Bytecode/` - Binary `.phsb` serialization (4-section: constants → variables → functions → instructions)
   - `IR/` - Assembly `.phir` format with inline comments
   - `Cpp/` - Generates C++ headers with embedded bytecode
4. **ISA/** - Opcode definitions (stack-based and register-based operations)

### Compilation & Execution
- **Compiler/** - CLI tools
  - `Phasor/Compiler` - Compiles `.phs` → `.phsb` or `.phir`
  - `Phasor/CppCompiler` - `.phs` → `.phsb` (in-memory) → C++ header + main.cpp stub → object file → linked executable
  - `Shared/Assembler` - Assembles `.phir` → `.phsb`
  - `Shared/Disassembler` - Disassembles `.phsb` → `.phir`
- **Frontend/** - High-level API (runScript, runRepl)
- **Runtime/** - VM, standard library, FFI layer

### Tools & Interfaces
- **Executable/** - Entry points for all CLI tools (Main, Compiler, Runtime, Repl, LSP)
- **LSP/** - Language Server Protocol (hover, go-to-definition, diagnostics)
- **Extensions/** - TextMate grammars, VS Code extension, Python bytecode module
- **Bindings/** - OS API bindings (Windows, POSIX)

Follow `.clang-format` for code style, add tests for new features, and update documentation.
