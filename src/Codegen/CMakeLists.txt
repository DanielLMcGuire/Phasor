set(CODEGEN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Codegen/CodeGen.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeDeserializer.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/IR/PhasorIR.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Cpp/CppCodeGenerator.cpp
)

set(CODEGEN_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Codegen/CodeGen.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeSerializer.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeDeserializer.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/IR/PhasorIR.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Cpp/CppCodeGenerator.hpp
)

add_library(PhasorCodegen STATIC ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})