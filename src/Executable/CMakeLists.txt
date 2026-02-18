add_library(phasor_native_runtime SHARED
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Phasor/NativeRuntimeDll.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Phasor/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)

add_library(phasor_native_runtime_static STATIC
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Phasor/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/NativeRuntime.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime_static PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)

add_executable(phasor_main 
    ${CMAKE_SOURCE_DIR}/src/Executable/Main/Phasor/Phasor.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Main/Phasor/Phasor_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/ScriptingRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Shared/BinaryRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Repl/Phasor/Repl.cpp
)
target_link_libraries(phasor_main PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
set_target_properties(phasor_main PROPERTIES OUTPUT_NAME phasor)

add_executable(pulsar_main 
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Pulsar/ScriptingRuntime.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Pulsar/ScriptingRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Pulsar/ScriptingRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Pulsar/ScriptingRuntime.hpp
)
target_link_libraries(pulsar_main PhasorRuntime pulsar_language PhasorCodegen PulsarFrontend)
set_target_properties(pulsar_main PROPERTIES OUTPUT_NAME pulsar)

add_executable(phasor_repl
    ${CMAKE_SOURCE_DIR}/src/Executable/Repl/Phasor/Repl.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Repl/Phasor/Repl_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Repl/Phasor/Repl.cpp
    ${CMAKE_SOURCE_DIR}/src/Repl/Phasor/Repl.hpp
)
target_link_libraries(phasor_repl PhasorRuntime phasor_language PhasorFrontend)
set_target_properties(phasor_repl PROPERTIES OUTPUT_NAME phasorrepl)

add_executable(phasor_compiler
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Phasor/Compiler.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Phasor/Compiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/Compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/Compiler.hpp
)
target_link_libraries(phasor_compiler phasor_language PhasorCodegen)
set_target_properties(phasor_compiler PROPERTIES OUTPUT_NAME phasorcompiler)

add_executable(pulsar_compiler
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Pulsar/Compiler.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Pulsar/Compiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Pulsar/Compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Pulsar/Compiler.hpp
)
target_link_libraries(pulsar_compiler pulsar_language PhasorCodegen)
set_target_properties(pulsar_compiler PROPERTIES OUTPUT_NAME pulsarcompiler)

add_executable(phasor_disasm
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Shared/Disassembler.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Shared/Disassembler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Shared/Disassembler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Shared/Disassembler.hpp
)
target_link_libraries(phasor_disasm phasor_language PhasorCodegen)
set_target_properties(phasor_disasm PROPERTIES OUTPUT_NAME phasordecomp)

add_executable(phasor_cpp_compiler
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Phasor/CppCompiler.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Compiler/Phasor/CppCompiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/CppCompiler.cpp
    ${CMAKE_SOURCE_DIR}/src/Compiler/Phasor/CppCompiler.hpp
)
target_link_libraries(phasor_cpp_compiler phasor_language PhasorCodegen)
set_target_properties(phasor_cpp_compiler PROPERTIES OUTPUT_NAME phasornative)

add_executable(phasor_runtime_exe
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Shared/BinaryRuntime.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Shared/BinaryRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Shared/BinaryRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Shared/BinaryRuntime.hpp
)
target_link_libraries(phasor_runtime_exe PhasorRuntime PhasorCodegen)
set_target_properties(phasor_runtime_exe PROPERTIES OUTPUT_NAME phasorvm)

add_executable(phasor_interpreter
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Phasor/ScriptingRuntime.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/Runtime/Phasor/ScriptingRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/ScriptingRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Phasor/ScriptingRuntime.hpp
)
target_link_libraries(phasor_interpreter PhasorRuntime phasor_language PhasorCodegen PhasorFrontend)
set_target_properties(phasor_interpreter PROPERTIES OUTPUT_NAME phasorjit)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/src/utils/Shell.hpp
    COMMAND $<TARGET_FILE:phasor_cpp_compiler> ${CMAKE_SOURCE_DIR}/src/utils/Shell.phs -o ${CMAKE_BINARY_DIR}/src/utils/Shell.hpp -H --nologo
    DEPENDS ${CMAKE_SOURCE_DIR}/src/utils/Shell.phs phasor_cpp_compiler
    COMMENT "Building PHS bytecode src/Library/utils/Shell.hpp"
)

add_executable(phasor_shell
    ${CMAKE_SOURCE_DIR}/src/Executable/utils/Shell.rc
    ${CMAKE_SOURCE_DIR}/src/Executable/utils/shell_main.cpp
    ${CMAKE_BINARY_DIR}/src/utils/Shell.hpp
    ${CMAKE_SOURCE_DIR}/src/utils/Shell.phs
)
target_include_directories(phasor_shell PRIVATE ${CMAKE_BINARY_DIR}/src/utils)
target_link_libraries(phasor_shell phasor_native_runtime)
set_target_properties(phasor_shell PROPERTIES OUTPUT_NAME phash)

if(WIN32)
    add_executable(phasor_winapi_gen
        ${CMAKE_SOURCE_DIR}/src/Executable/Bindings/win32/bindgen_main.cpp
    )
    set_target_properties(phasor_winapi_gen PROPERTIES OUTPUT_NAME winapi-gen-phs)

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/phasor_winapi_bindings.cpp
        COMMAND $<TARGET_FILE:phasor_winapi_gen> ${CMAKE_SOURCE_DIR}/scripts/winapi.h -o ${CMAKE_BINARY_DIR}/phasor_winapi_bindings.cpp
        DEPENDS ${CMAKE_SOURCE_DIR}/scripts/winapi.h phasor_winapi_gen
        COMMENT "Generating win32 bindings phasor_winapi_bindings.cpp"
    )

    add_library(phasor_winapi_bindings SHARED
        ${CMAKE_BINARY_DIR}/phasor_winapi_bindings.cpp
        ${CMAKE_SOURCE_DIR}/src/Bindings/win32/handle.hpp
        ${CMAKE_SOURCE_DIR}/src/Executable/Bindings/win32/main.rc
        ${CMAKE_SOURCE_DIR}/scripts/winapi.h
    )
    set_target_properties(phasor_winapi_bindings PROPERTIES 
        OUTPUT_NAME winapi_phs
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    )
    install(TARGETS phasor_winapi_bindings RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR})
else()
    add_library(phasor_posix_bindings SHARED
        ${CMAKE_SOURCE_DIR}/src/Executable/Bindings/posix/main.c
    )
    set_target_properties(phasor_posix_bindings PROPERTIES 
        OUTPUT_NAME posix_phs
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    )
    install(TARGETS phasor_posix_bindings
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
    )
endif()

add_subdirectory(utils)