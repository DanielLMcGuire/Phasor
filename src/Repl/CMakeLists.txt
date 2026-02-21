add_library(phasor_repl_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Repl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Repl.hpp
)

target_link_libraries(phasor_repl_lib PUBLIC PhasorCodegen PhasorRuntime phasor_language PhasorFrontend)