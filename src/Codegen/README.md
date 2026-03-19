`CodeGen.hpp/.cpp` - Code generator. Walks the AST and emits Instruction objects into a Bytecode struct (constant pool, variable map, function entries, struct metadata). Does constant folding on literal binary expressions and basic type inference to pick integer vs. float opcodes. Uses a small register allocator for binary expressions, with loop context stacks for break/continue jump patching.

`Bytecode/` - Binary `.phsb` serializer/deserializer. 4-section layout: constants → variables → functions → instructions, with a CRC32 integrity check in the header. Also has a python module in `../Extensions`.

`IR/` — Assembly `.phir` serializer/deserializer. Includes inline comments in the output (e.g. ; var=x, ; const[0]="hello").

`Cpp/` — Generates a C++ header with the bytecode embedded as a `constexpr unsigned char[]`, placed in a named executable section (.phsb). Used by the native compiler output and is also supported by the bytecode Python module.