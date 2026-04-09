| Folder     | Description                                                    |
| ---------- | -------------------------------------------------------------- |
| AST        | Abstract Syntax Tree node and Token definitions.               |
| Bindings   | Phasor FFI Bindings for OS APIs. (win32, posix)                |
| Codegen    | Phasor VM Code Generation. (AST -> ISA)                        |
| Compiler   | Phasor / Pulsar Compilers. (Wrapper for codegen and language)  |
| Executable | Frontfacing code (BIN+LIB). Mirrors this layout well.          |
| Extensions | Support for third-party apps/tools.                            |
| Frontend   | Higher level wrappers around core subsystems for the languages.|
| ISA        | The OpCode set.                                                |
| Language   | Lexer / Parser Implementations.                                |
| Repl       | REPL Code for Phasor / Pulsar.                                 |
| Runtime    | Contains code for the VM, STDLIB, FFI + high level wrappers.   |
| **/core    | Low level (C/Assembly) implementations.                        |
