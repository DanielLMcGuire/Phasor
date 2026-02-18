add_library(phasor_compiler_lib STATIC
    Phasor/Compiler.cpp
    Phasor/Compiler.hpp
)

target_link_libraries(phasor_compiler_lib phasor_language PhasorCodegen PhasorFrontend)

add_library(pulsar_compiler_lib STATIC
    Pulsar/Compiler.cpp
    Pulsar/Compiler.hpp
)

target_link_libraries(pulsar_compiler_lib pulsar_language PhasorCodegen PhasorFrontend)