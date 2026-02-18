cmake_minimum_required(VERSION 3.10)
project(Phasor VERSION 2.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Default install prefix" FORCE)
endif()

if(WIN32)
    set(PLUGIN_INSTALL_DIR "bin/plugins")
elseif(APPLE)
    set(PLUGIN_INSTALL_DIR "Library/Application Support/org.Phasor.Phasor/plugins")
else()
    set(PLUGIN_INSTALL_DIR "opt/Phasor/plugins")
endif()

include_directories(include src ${CMAKE_CURRENT_BINARY_DIR})

option(ASSEMBLY "Enable assembly for core code" ON)

if(WIN32 AND ASSEMBLY)
    enable_language(RC)
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE
        "/O2 /Oi /Ot /GL /Gy /MT /fp:precise /arch:AVX2 /Qspectre-"
    )
    set(CMAKE_CXX_FLAGS_RELEASE
        "/O2 /Oi /Ot /GL /Gy /MT /fp:precise /arch:AVX2 /EHsc /permissive- /DNOMINMAX"
    )
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "/LTCG /OPT:REF /OPT:ICF"
    )
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
        "/LTCG /OPT:REF /OPT:ICF"
    )
    set(CMAKE_C_FLAGS_DEBUG "/Od /Zi /fp:strict")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /fp:strict /EHsc /DNOMINMAX")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")
else()
    set(COMMON_OPT
        "-O3 -flto -funroll-loops -fomit-frame-pointer -Wno-missing-field-initializers"
    )
    set(COMMON_FP
        "-ffast-math"
    )
    set(COMMON_WARN
        "-Wall -Wextra -pedantic"
    )
    set(COMMON_CXX_LANG
        "-fexceptions -frtti"
    )
    
    set(CMAKE_C_FLAGS_RELEASE
        "${COMMON_OPT} ${COMMON_FP} -march=native"
    )

    set(CMAKE_CXX_FLAGS_RELEASE
        "${COMMON_OPT} ${COMMON_FP} ${COMMON_CXX_LANG} ${COMMON_WARN} -march=native"
    )
    
    if(APPLE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-dead_strip")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-Wl,-dead_strip")
    elseif(WIN32)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "")
    else()
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
    endif()
    
    set(CMAKE_C_FLAGS_DEBUG
        "-O0 -g -fno-omit-frame-pointer"
    )
    set(CMAKE_CXX_FLAGS_DEBUG
        "-O0 -g -fno-omit-frame-pointer -fexceptions -frtti"
    )
endif()

if(WIN32)
    if(ASSEMBLY)
        enable_language(ASM_MASM)
    endif()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/crt.c)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/crt.c)
    else()
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/windows_x86_64.asm)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/windows_x86_64.asm)
    endif()
elseif(APPLE OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    if(ASSEMBLY)
        enable_language(ASM)
    endif()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/crt.c)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/crt.c)
    else()
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/bsd_x86_64.s)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/bsd_x86_64.s)
    endif()
elseif(UNIX)
    if(ASSEMBLY)
        enable_language(ASM)
    endif()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/crt.c)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/crt.c)
    else()
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/linux_x86_64.s)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/linux_x86_64.s)
    endif()
endif()

set(RUNTIME_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Native.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Operations.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Register.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Stack.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Utility.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Variables.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/ffi.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/api.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/IO/IO.c
)
if(ASSEMBLY)
    list(APPEND RUNTIME_SOURCES ${ARITHMETIC_ASM} ${LOGICAL_ASM})
else()
    list(APPEND RUNTIME_SOURCES
        ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/arithmetic/crt.c
        ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/logical/crt.c
    )
endif()

set(RUNTIME_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Runtime/Value.hpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/VM.hpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/core.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/arithmetic/arithmetic.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/logical/logical.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/IO/IO.h
    ${CMAKE_SOURCE_DIR}/src/sscanf.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/ffi.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
    ${CMAKE_SOURCE_DIR}/src/AST/AST.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

set(STDLIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/file_properties.c
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/system.c
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/StdLib.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/file.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/io.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/regex.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/math.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/string.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/system.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/typeconv.cpp
)

set(STDLIB_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/file_properties.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/system.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/StdLib.hpp
)

set(PHASOR_LANGUAGE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Lexer/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Parser/Parser.cpp
)

set(PHASOR_LANGUAGE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Lexer/Lexer.hpp
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Parser/Parser.hpp
)

set(PULSAR_LANGUAGE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Lexer/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Parser/Parser.cpp
)

set(PULSAR_LANGUAGE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Lexer/Lexer.hpp
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Parser/Parser.cpp
)

set(CODEGEN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Codegen/CodeGen.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeDeserializer.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/IR/PhasorIR.cpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Cpp/CppCodeGenerator.cpp
)

set(CODEGEN_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Codegen/CodeGen.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeSerializer.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Bytecode/BytecodeDeserializer.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/IR/PhasorIR.hpp
    ${CMAKE_SOURCE_DIR}/src/Codegen/Cpp/CppCodeGenerator.hpp
)

add_library(PhasorRuntime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS} ${STDLIB_SOURCES} ${STDLIB_HEADERS})

add_library(phasor_language STATIC ${PHASOR_LANGUAGE_SOURCES} ${PHASOR_LANGUAGE_HEADERS})

add_library(pulsar_language STATIC ${PULSAR_LANGUAGE_SOURCES} ${PULSAR_LANGUAGE_HEADERS})

add_library(PhasorCodegen STATIC ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})

add_library(PhasorFrontend STATIC 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Phasor/Frontend.cpp 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Phasor/Frontend.hpp
)
target_link_libraries(PhasorFrontend PUBLIC PhasorCodegen PhasorRuntime)

add_library(PulsarFrontend STATIC 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Pulsar/Frontend.cpp
    ${CMAKE_SOURCE_DIR}/src/Frontend/Pulsar/Frontend.hpp
)
target_link_libraries(PulsarFrontend PUBLIC PhasorCodegen PhasorRuntime)

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
        ${CMAKE_SOURCE_DIR}/src/Executable/Bindings/main.c
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

set(TARGETS cat cp echo ls mv rm touch)
set(BIN_RAW "src/utils/coreutils")
set(BIN_DIR "${CMAKE_BINARY_DIR}/${BIN_RAW}")

foreach(tgt ${TARGETS})
    set(PHS "${CMAKE_SOURCE_DIR}/src/utils/coreutils/${tgt}.phs")
    set(HEADER "${BIN_DIR}/${tgt}.hpp")
    set(GEN_CPP "${BIN_DIR}/${tgt}.cpp")
    set(PATH_COUT "${BIN_RAW}/${tgt}.hpp")


    file(MAKE_DIRECTORY ${BIN_DIR})

    add_custom_command(
        OUTPUT ${HEADER}
        COMMAND $<TARGET_FILE:phasor_cpp_compiler> ${PHS} -o ${HEADER} -H --nologo
        DEPENDS ${PHS} phasor_cpp_compiler
        COMMENT "Building PHS object ${PATH_COUT}"
    )

    configure_file(
        "${CMAKE_SOURCE_DIR}/src/Executable/utils/coreutils/coreutils_main.in.cpp"
        "${GEN_CPP}"
        @ONLY
    )

    add_executable(${tgt}
        ${GEN_CPP}
        src/Executable/utils/coreutils/${tgt}.rc
        ${HEADER}
        ${PHS}
    )
    target_include_directories(${tgt} PRIVATE ${BIN_DIR})
    target_link_libraries(${tgt} phasor_native_runtime)
    set_target_properties(${tgt} PROPERTIES OUTPUT_NAME "${tgt}-phs")
    if(WIN32)
        install(TARGETS ${tgt}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
        )
    elseif(APPLE)
        install(TARGETS ${tgt}
            RUNTIME DESTINATION usr/local/bin
            LIBRARY DESTINATION usr/local/lib
            ARCHIVE DESTINATION usr/local/lib
            FRAMEWORK DESTINATION frameworks
        )
    else()
        install(TARGETS ${tgt}
            RUNTIME DESTINATION usr/bin
            LIBRARY DESTINATION usr/lib
            ARCHIVE DESTINATION usr/lib
        )
    endif()
endforeach()

if(WIN32)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/" 
        DESTINATION bin 
        FILES_MATCHING 
        PATTERN "*.pdb" 
        PATTERN "CMakeFiles" EXCLUDE
        PATTERN "src" EXCLUDE
	    PATTERN "winapi-gen-phs.pdb" EXCLUDE
		PATTERN "clocf.pdb" EXCLUDE
    )
    install(TARGETS
        phasor_main
        pulsar_main
        phasor_compiler
        pulsar_compiler
        phasor_disasm
        phasor_cpp_compiler
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
        phasor_repl
        phasor_interpreter
        phasor_shell
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    install(FILES
        ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
        ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
        DESTINATION include
    )
elseif(APPLE)
    install(TARGETS
        phasor_main
        pulsar_main
        phasor_compiler
        pulsar_compiler
        phasor_disasm
        phasor_cpp_compiler
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
        phasor_repl
        phasor_interpreter
        phasor_shell
        RUNTIME DESTINATION usr/local/bin
        LIBRARY DESTINATION usr/local/lib
        ARCHIVE DESTINATION usr/local/lib
        FRAMEWORK DESTINATION frameworks
    )
    install(FILES
        ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
        ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
        DESTINATION usr/local/include
    )
else()
    install(TARGETS
        phasor_main
        pulsar_main
        phasor_compiler
        pulsar_compiler
        phasor_disasm
        phasor_cpp_compiler
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
        phasor_repl
        phasor_interpreter
        phasor_shell
        RUNTIME DESTINATION usr/bin
        LIBRARY DESTINATION usr/lib
        ARCHIVE DESTINATION usr/lib
    )
    install(FILES
        ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
        ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
        DESTINATION usr/include
    )
endif()
