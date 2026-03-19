`Phasor/`:
-   `Compiler/` - CLI wrapper that takes a Phasor `.phs` source file and outputs either `.phsb` bytecode or `.phir` IR, depending on flags.
-   `CppCompiler/` - CLI wrapper for the native compilation path: generate a C++ header (via CppCodeGenerator), compile it to an object file, and link it against the phasor-runtime DLL. Supports header-only, object-only, and generate-only modes.

`Pulsar/` - Mirror of Phasor/Compiler for the Pulsar language (pulsar namespace).

`Shared/` - Language-agnostic tools.
-   `Assembler` - Assembles `.phir` IR into `.phsb` bytecode.
-   `Disassembler` - Takes a `.phsb` binary and outputs `.phir` IR.