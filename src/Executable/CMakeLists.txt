add_library(phasor_native_runtime SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/NativeRuntimeDll.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime phasor_native_runtime_lib)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)

add_library(phasor_native_runtime_static STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime_static phasor_native_runtime_lib)
set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)

add_executable(phasor_main 
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Repl/Phasor/Repl.cpp
)
target_link_libraries(phasor_main phasor_scripting_runtime_lib phasor_binary_runtime_lib PhasorFrontend)
set_target_properties(phasor_main PROPERTIES OUTPUT_NAME phasor)

add_executable(pulsar_main 
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Pulsar/ScriptingRuntime.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Pulsar/ScriptingRuntime_main.cpp
)
target_link_libraries(pulsar_main pulsar_scripting_runtime_lib PulsarFrontend)
set_target_properties(pulsar_main PROPERTIES OUTPUT_NAME pulsar)

add_executable(phasor_repl
    ${CMAKE_CURRENT_SOURCE_DIR}/Repl/Phasor/Repl.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Repl/Phasor/Repl_main.cpp
)
target_link_libraries(phasor_repl phasor_repl_lib)
set_target_properties(phasor_repl PROPERTIES OUTPUT_NAME phasorrepl)

add_executable(phasor_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler_main.cpp
)
target_link_libraries(phasor_compiler phasor_compiler_lib)
set_target_properties(phasor_compiler PROPERTIES OUTPUT_NAME phasorcompiler)

add_executable(pulsar_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Pulsar/Compiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Pulsar/Compiler_main.cpp
)
target_link_libraries(pulsar_compiler pulsar_compiler_lib)
set_target_properties(pulsar_compiler PROPERTIES OUTPUT_NAME pulsarcompiler)

add_executable(phasor_lsp
    ${CMAKE_CURRENT_SOURCE_DIR}/LSP/Phasor/LSP_main.cpp
)
target_link_libraries(phasor_lsp phasor_lsp_lib)
set_target_properties(phasor_lsp PROPERTIES OUTPUT_NAME phasor-lsp)

add_executable(phasor_disasm
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Disassembler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Disassembler_main.cpp
)
target_link_libraries(phasor_disasm phasor_disasm_lib)
set_target_properties(phasor_disasm PROPERTIES OUTPUT_NAME phasordecomp)

add_executable(phasor_cxx_transpiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/CppCompiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/CppCompiler_main.cpp
)
target_link_libraries(phasor_cxx_transpiler phasor_cxx_transpiler_lib)
set_target_properties(phasor_cxx_transpiler PROPERTIES OUTPUT_NAME phasornative)

add_executable(phasor_runtime_exe
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/BinaryRuntime.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/BinaryRuntime_main.cpp
)
target_link_libraries(phasor_runtime_exe phasor_binary_runtime_lib)
set_target_properties(phasor_runtime_exe PROPERTIES OUTPUT_NAME phasorvm)

add_executable(phasor_interpreter
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/ScriptingRuntime.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/ScriptingRuntime_main.cpp
)
target_link_libraries(phasor_interpreter phasor_scripting_runtime_lib PhasorFrontend)
set_target_properties(phasor_interpreter PROPERTIES OUTPUT_NAME phasorjit)

if(WIN32)
    add_executable(phasor_winapi_gen
        ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/win32/bindgen_main.cpp
    )
    set_target_properties(phasor_winapi_gen PROPERTIES OUTPUT_NAME winapi-gen-phs)

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/Executable/Bindings/win32/winapi_main.cpp
        COMMAND $<TARGET_FILE:phasor_winapi_gen> ${CMAKE_SOURCE_DIR}/scripts/winapi.h -o ${CMAKE_CURRENT_BINARY_DIR}/src/Executable/Bindings/win32/winapi_main.cpp
        DEPENDS ${CMAKE_SOURCE_DIR}/scripts/winapi.h phasor_winapi_gen
        COMMENT "Generating win32 bindings src/Executable/Bindings/win32/winapi_main.cpp"
    )

    add_library(phasor_winapi_bindings SHARED
        ${CMAKE_CURRENT_BINARY_DIR}/src/Executable/Bindings/win32/winapi_main.cpp
        ${CMAKE_SOURCE_DIR}/src/Bindings/win32/handle.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/win32/main.rc
        ${CMAKE_SOURCE_DIR}/scripts/winapi.h
    )
    set_target_properties(phasor_winapi_bindings PROPERTIES 
        OUTPUT_NAME winapi_phs
    )
    install(TARGETS phasor_winapi_bindings RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR})
else()
    add_library(phasor_posix_bindings SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/posix/main.c
    )
    set_target_properties(phasor_posix_bindings PROPERTIES 
        OUTPUT_NAME posix_phs
    )
    install(TARGETS phasor_posix_bindings
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
    )
endif()

add_subdirectory(utils)