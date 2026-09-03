add_library(phasor_c_transpiler_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/CCompiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/CCompiler.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/C/CCodeGenerator.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/C/CCodeGenerator.cpp
)

target_link_libraries(phasor_c_transpiler_lib phasor_native_runtime)