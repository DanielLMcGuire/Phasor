add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/phasor_help.hpp
    COMMAND $<TARGET_FILE:phasor_cxx_transpiler> ${CMAKE_CURRENT_SOURCE_DIR}/phasor_help.phs -o ${CMAKE_CURRENT_BINARY_DIR}/phasor_help.hpp -H
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/phasor_help.phs phasor_cxx_transpiler
    COMMENT "Building PHS bytecode phasor_help.hpp"
)

add_executable(phasor_help
    phasor_help.rc
    phasor_help_main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/phasor_help.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/phasor_help.phs
)
target_link_libraries(phasor_help phasor_native_runtime)
include_directories(phasor_help ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(phasor_help PROPERTIES OUTPUT_NAME phasor-help)