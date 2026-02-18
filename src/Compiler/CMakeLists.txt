add_library(phasor_compiler_lib STATIC
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/Compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/Compiler.hpp
)

target_link_libraries(phasor_compiler_lib phasor_language PhasorCodegen PhasorFrontend)

add_library(pulsar_compiler_lib STATIC
    ${CMAKE_SOURCE_DIR}/src/Compiler/Pulsar/Compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Pulsar/Compiler.hpp
)

target_link_libraries(pulsar_compiler_lib pulsar_language PhasorCodegen PhasorFrontend)