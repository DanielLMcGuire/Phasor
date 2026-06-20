add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/phasor_help.hpp
    COMMAND $<TARGET_FILE:phasor_cxx_transpiler> "${CMAKE_CURRENT_SOURCE_DIR}/main.phs" -o "${CMAKE_CURRENT_BINARY_DIR}/phasor_help.hpp" -H -I "${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include"
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/phasor_help.phs 
        phasor_cxx_transpiler
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/io.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/src/std/io.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/sys.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/src/std/sys.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/str.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/type.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/meta.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/src/std/meta.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/file.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/src/std/file.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/mem.phs
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/std/main.phs
    COMMENT "Building PHS bytecode phasor_help.hpp"
)

add_executable(phasor_help
    phasor_help.rc
    phasor_help_main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/phasor_help.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/phasor_help.phs
    ${CMAKE_CURRENT_SOURCE_DIR}/main.phs
)
target_link_libraries(phasor_help phasor_native_runtime)
include_directories(phasor_help ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(phasor_help PROPERTIES OUTPUT_NAME phasor-help)