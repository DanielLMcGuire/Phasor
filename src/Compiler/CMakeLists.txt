add_library(phasor_compiler_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Compiler.hpp
)

target_link_libraries(phasor_compiler_lib phasor_language PhasorCodegen)

add_library(phasor_cxx_transpiler_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/CppCompiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/CppCompiler.hpp
)

target_link_libraries(phasor_cxx_transpiler_lib phasor_language PhasorCodegen)

add_library(pulsar_compiler_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Compiler.hpp
)

target_link_libraries(pulsar_compiler_lib pulsar_language PhasorCodegen)

add_library(phasor_disasm_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/Disassembler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/Disassembler.hpp
)

target_link_libraries(phasor_disasm_lib PhasorCodegen)