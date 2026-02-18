set(CODEGEN_SOURCES
    CodeGen.cpp
    Bytecode/BytecodeSerializer.cpp
    Bytecode/BytecodeDeserializer.cpp
    IR/PhasorIR.cpp
    Cpp/CppCodeGenerator.cpp
)

set(CODEGEN_HEADERS
    CodeGen.hpp
    Bytecode/BytecodeSerializer.hpp
    Bytecode/BytecodeDeserializer.hpp
    IR/PhasorIR.hpp
    Cpp/CppCodeGenerator.hpp
)

add_library(PhasorCodegen STATIC ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})