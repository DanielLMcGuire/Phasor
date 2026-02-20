set(CODEGEN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CodeGen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Bytecode/BytecodeSerializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Bytecode/BytecodeDeserializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IR/PhasorIR.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Cpp/CppCodeGenerator.cpp
)

set(CODEGEN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/CodeGen.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Bytecode/BytecodeSerializer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Bytecode/BytecodeDeserializer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IR/PhasorIR.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Cpp/CppCodeGenerator.hpp
)

add_library(PhasorCodegen STATIC ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})