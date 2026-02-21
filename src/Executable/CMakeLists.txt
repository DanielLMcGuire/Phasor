add_library(phasor_native_runtime SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/NativeRuntimeDll.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)

add_library(phasor_native_runtime_static STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime_static PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)

add_executable(phasor_main 
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/ScriptingRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Shared/BinaryRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Repl/Phasor/Repl.cpp
)
target_link_libraries(phasor_main PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
set_target_properties(phasor_main PROPERTIES OUTPUT_NAME phasor)

add_executable(pulsar_main 
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Pulsar/ScriptingRuntime.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Pulsar/ScriptingRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Pulsar/ScriptingRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Pulsar/ScriptingRuntime.hpp
)
target_link_libraries(pulsar_main PhasorRuntime pulsar_language PhasorCodegen PulsarFrontend)
set_target_properties(pulsar_main PROPERTIES OUTPUT_NAME pulsar)

add_executable(phasor_repl
    ${CMAKE_CURRENT_SOURCE_DIR}/Repl/Phasor/Repl.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Repl/Phasor/Repl_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Repl/Phasor/Repl.cpp
    ${CMAKE_SOURCE_DIR}/src/Repl/Phasor/Repl.hpp
)
target_link_libraries(phasor_repl PhasorRuntime phasor_language PhasorFrontend)
set_target_properties(phasor_repl PROPERTIES OUTPUT_NAME phasorrepl)

add_executable(phasor_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/Compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/Compiler.hpp
)
target_link_libraries(phasor_compiler phasor_language PhasorCodegen)
set_target_properties(phasor_compiler PROPERTIES OUTPUT_NAME phasorcompiler)

add_executable(pulsar_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Pulsar/Compiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Pulsar/Compiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Pulsar/Compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Pulsar/Compiler.hpp
)
target_link_libraries(pulsar_compiler pulsar_language PhasorCodegen)
set_target_properties(pulsar_compiler PROPERTIES OUTPUT_NAME pulsarcompiler)

add_executable(phasor_lsp
    ${CMAKE_CURRENT_SOURCE_DIR}/LSP/Phasor/LSP_main.cpp
    ${CMAKE_SOURCE_DIR}/src/LSP/Phasor/LSP.cpp
    ${CMAKE_SOURCE_DIR}/src/LSP/Phasor/LSP.hpp
)
target_link_libraries(phasor_lsp phasor_language PhasorCodegen)
set_target_properties(phasor_lsp PROPERTIES OUTPUT_NAME phasor-lsp)

add_executable(phasor_disasm
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Disassembler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Disassembler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Shared/Disassembler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Shared/Disassembler.hpp
)
target_link_libraries(phasor_disasm phasor_language PhasorCodegen)
set_target_properties(phasor_disasm PROPERTIES OUTPUT_NAME phasordecomp)

add_executable(phasor_cxx_transpiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/CppCompiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/CppCompiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/CppCompiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/CppCompiler.hpp
)
target_link_libraries(phasor_cxx_transpiler phasor_language PhasorCodegen)
set_target_properties(phasor_cxx_transpiler PROPERTIES OUTPUT_NAME phasornative)

add_executable(phasor_runtime_exe
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/BinaryRuntime.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/BinaryRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Shared/BinaryRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Shared/BinaryRuntime.hpp
)
target_link_libraries(phasor_runtime_exe PhasorRuntime PhasorCodegen)
set_target_properties(phasor_runtime_exe PROPERTIES OUTPUT_NAME phasorvm)

add_executable(phasor_interpreter
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/ScriptingRuntime.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Phasor/ScriptingRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/ScriptingRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/ScriptingRuntime.hpp
)
target_link_libraries(phasor_interpreter PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
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