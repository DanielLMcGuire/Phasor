add_library(phasor_c_transpiler_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/CCompiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/CCompiler.hpp
)

target_link_libraries(phasor_c_transpiler_lib phasor_language PhasorCodegen)

add_library(phasor_disasm_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/Disassembler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/Disassembler.hpp
)

target_link_libraries(phasor_disasm_lib PhasorCodegen)

add_library(phasor_asm_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/Assembler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/Assembler.hpp
)

target_link_libraries(phasor_asm_lib PhasorCodegen)