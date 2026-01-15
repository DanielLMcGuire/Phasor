cmake_minimum_required(VERSION 3.10)
project(Phasor VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(include)
include_directories(src)

# Set flags

option(ASSEMBLY "Enable assembly for core code" ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox /DNOMINMAX /EHsc /fp:precise /arch:AVX2")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")

    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /DNOMINMAX /EHsc /fp:fast")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Ox /fp:precise /arch:AVX2")
    set(CMAKE_C_FLAGS_DEBUG "/Od /Zi /fp:fast")

else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -fno-stack-protector")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")

    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -fno-stack-protector")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer")
endif()

if(MSVC)
    enable_language(ASM_MASM)
    enable_language(RC)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/windows_aarch64.asm)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/windows_aarch64.asm)
        set(IO_ASM src/Runtime/VM/core/IO/windows_aarch64.asm)
    else()
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/windows_x86_64.asm)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/windows_x86_64.asm)
        set(IO_ASM src/Runtime/VM/core/IO/windows_x86_64.asm)
    endif()

elseif(APPLE OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    # FreeBSD
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/darwin_bsd_aarch64.s)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/darwin_bsd_aarch64.s)
        set(IO_ASM src/Runtime/VM/core/IO/darwin_bsd_aarch64.s)
    else()
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/darwin_bsd_x86_64.s)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/darwin_bsd_x86_64.s)
        set(IO_ASM src/Runtime/VM/core/IO/darwin_bsd_x86_64.s)
    endif()
elseif(UNIX)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/linux_aarch64.s)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/linux_aarch64.s)
        set(IO_ASM src/Runtime/VM/core/IO/linux_aarch64.s)
    else()
        set(ARITHMETIC_ASM src/Runtime/VM/core/arithmetic/linux_x86_64.s)
        set(LOGICAL_ASM src/Runtime/VM/core/logical/linux_x86_64.s)
        set(IO_ASM src/Runtime/VM/core/IO/linux_x86_64.s)
    endif()
endif()


# Source files
set(RUNTIME_SOURCES
    src/Runtime/VM/VM.cpp
)

if(ASSEMBLY)
    list(APPEND RUNTIME_SOURCES
        ${ARITHMETIC_ASM}
        ${LOGICAL_ASM}
        ${IO_ASM}
    )
else()
    list(APPEND RUNTIME_SOURCES
        src/Runtime/VM/core/arithmetic/crt.c
        src/Runtime/VM/core/logical/crt.c
        src/Runtime/VM/core/IO/crt.c
    )
endif()

set(STDLIB_SOURCES
    src/Runtime/Stdlib/core/file_properties.c
    src/Runtime/Stdlib/StdLib.cpp
    src/Runtime/Stdlib/file.cpp
    src/Runtime/Stdlib/io.cpp
    src/Runtime/Stdlib/regex.cpp
    src/Runtime/Stdlib/math.cpp
    src/Runtime/Stdlib/string.cpp
    src/Runtime/Stdlib/system.cpp
    src/Runtime/Stdlib/typeconv.cpp
)

set(STDLIB_HEADERS
    src/Runtime/Stdlib/core/file_properties.h
    src/Runtime/Stdlib/StdLib.hpp
)

if(WIN32)
    list(APPEND STDLIB_SOURCES
        src/Runtime/Stdlib/windows/windows.cpp
        src/Runtime/Stdlib/windows/vhandle.cpp
        src/Runtime/Stdlib/windows/winType.cpp
        src/Runtime/Stdlib/windows/winuser.cpp
    )
    list(APPEND STDLIB_HEADERS
        src/Runtime/Stdlib/windows/windows.hpp
        src/Runtime/Stdlib/windows/vhandle.hpp
        src/Runtime/Stdlib/windows/winType.hpp
        src/Runtime/Stdlib/windows/winuser.hpp
    )
endif()

set(RUNTIME_HEADERS
    src/Runtime/Value.hpp
    src/Runtime/VM/VM.hpp
    "src/Runtime/VM/core/core.h"
    src/Runtime/VM/core/arithmetic/arithmetic.h
    src/Runtime/VM/core/logical/logical.h
    src/Runtime/VM/core/IO/IO.h
    src/sscanf.h
)

set(INTERPRETER_SOURCES
    src/Backend/Lexer/Lexer.cpp
    src/Backend/Parser/Parser.cpp
)

set(INTERPRETER_HEADERS
    src/Backend/Lexer/Lexer.hpp
    src/Backend/Parser/Parser.hpp
)

set(CODEGEN_SOURCES
    src/Codegen/CodeGen.cpp
    src/Codegen/Bytecode/BytecodeSerializer.cpp
    src/Codegen/Bytecode/BytecodeDeserializer.cpp
    src/Codegen/IR/PhasorIR.cpp
    src/Codegen/Cpp/CppCodeGenerator.cpp
)

set(CODEGEN_HEADERS
    src/Codegen/CodeGen.hpp
    src/Codegen/Bytecode/BytecodeSerializer.hpp
    src/Codegen/Bytecode/BytecodeDeserializer.hpp
    src/Codegen/IR/PhasorIR.hpp
    src/Codegen/Cpp/CppCodeGenerator.hpp
)

set(FRONTEND_SOURCES
    src/Frontend/Frontend.cpp
)

set(FRONTEND_HEADERS
    src/Frontend/Frontend.hpp
)

add_library(PhasorRuntime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS} ${STDLIB_SOURCES} ${STDLIB_HEADERS})

if(WIN32)
    target_link_libraries(PhasorRuntime ws2_32)
endif()

add_library(PhasorInterpreter STATIC ${INTERPRETER_SOURCES} ${INTERPRETER_HEADERS})

add_library(PhasorCodegen STATIC ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})

add_library(PhasorFrontend STATIC ${FRONTEND_SOURCES} ${FRONTEND_HEADERS})

# JIT Powered REPL
add_executable(phasor_repl
    src/App/Repl/Repl.rc
    src/App/Repl/Repl_main.cpp
    src/CLI/Repl/Repl.cpp
    src/CLI/Repl/Repl.hpp
)
target_link_libraries(phasor_repl PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_repl PROPERTIES OUTPUT_NAME phasorrepl)

# Script to bytecode binary compiler
add_executable(phasor_compiler  
    src/App/Compiler/Compiler.rc
    src/App/Compiler/Compiler_main.cpp
    src/CLI/Compiler/Compiler.cpp
    src/CLI/Compiler/Compiler.hpp
)
target_link_libraries(phasor_compiler PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_compiler PROPERTIES OUTPUT_NAME phasorcompiler)

# Script to C++ code compiler
add_executable(phasor_cpp_compiler
    src/App/Compiler/CppCompiler.rc
    src/App/Compiler/CppCompiler_main.cpp
    src/CLI/Compiler/CppCompiler.cpp
    src/CLI/Compiler/CppCompiler.hpp
)
target_link_libraries(phasor_cpp_compiler PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_cpp_compiler PROPERTIES OUTPUT_NAME phasornative)

# Execution engine for bytecode binaries
add_executable(phasor_runtime_exe
    src/App/Runtime/BinaryRuntime.rc
    src/App/Runtime/BinaryRuntime_main.cpp
    src/CLI/Runtime/BinaryRuntime.cpp
    src/CLI/Runtime/BinaryRuntime.hpp
)
target_link_libraries(phasor_runtime_exe PhasorRuntime PhasorCodegen PhasorFrontend)
set_target_properties(phasor_runtime_exe PROPERTIES OUTPUT_NAME phasorvm)

# DLL Execution engine for bytecode embedded native binaries
add_library(phasor_native_runtime SHARED 
    src/App/Runtime/NativeRuntimeDll.rc
    src/App/Runtime/NativeRuntime_library.cpp
    src/CLI/Runtime/NativeRuntime.cpp
    src/CLI/Runtime/NativeRuntime.hpp
)
target_link_libraries(phasor_native_runtime PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)

# DLL Execution engine for bytecode embedded native binaries
add_library(phasor_native_runtime_static STATIC 
    src/App/Runtime/NativeRuntime_library.cpp
    src/CLI/Runtime/NativeRuntime.cpp
    src/CLI/Runtime/NativeRuntime.hpp
)
target_link_libraries(phasor_native_runtime_static PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)

# JIT execution engine for scripts
add_executable(phasor_interpreter
    src/App/Runtime/ScriptingRuntime.rc
    src/App/Runtime/ScriptingRuntime_main.cpp
    src/CLI/Runtime/ScriptingRuntime.cpp
    src/CLI/Runtime/ScriptingRuntime.hpp
)
target_link_libraries(phasor_interpreter PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_interpreter PROPERTIES OUTPUT_NAME phasorjit)

# CLI Shell
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/src/CLI/utils/Shell.hpp
    COMMAND phasornative ${CMAKE_SOURCE_DIR}/src/CLI/utils/Shell.phs -o ${CMAKE_BINARY_DIR}/src/CLI/utils/Shell.hpp -H --nologo
    DEPENDS ${CMAKE_SOURCE_DIR}/src/CLI/utils/Shell.phs phasor_cpp_compiler
    COMMENT "Building PHS object src/CLI/utils/Shell.phs"
)
add_executable(phasor_shell
    src/App/utils/Shell.rc
    src/App/utils/Shell_main.cpp
    ${CMAKE_BINARY_DIR}/src/CLI/utils/Shell.hpp
)
target_include_directories(phasor_shell PRIVATE ${CMAKE_BINARY_DIR}/src/CLI/utils)
target_link_libraries(phasor_shell phasor_native_runtime)
set_target_properties(phasor_shell PROPERTIES OUTPUT_NAME shell)

install(TARGETS phasor_compiler phasor_cpp_compiler phasor_runtime_exe phasor_native_runtime phasor_native_runtime_static phasor_repl phasor_interpreter phasor_shell
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    FRAMEWORK DESTINATION "frameworks"
)

set(TARGETS
    cat
    cp
    echo
    ls
    mv
    rm
    touch
)

set(BIN_DIR "${CMAKE_BINARY_DIR}/src/CLI/utils/coreutils")

foreach(tgt ${TARGETS})
    set(PHS "${CMAKE_SOURCE_DIR}/src/CLI/utils/coreutils/${tgt}.phs")
    set(HEADER "${BIN_DIR}/${tgt}.hpp")
    set(GEN_CPP "${BIN_DIR}/${tgt}.cpp")

    file(MAKE_DIRECTORY ${BIN_DIR})

    add_custom_command(
        OUTPUT ${HEADER}
        COMMAND phasornative ${PHS} -o ${HEADER} -H --nologo
        DEPENDS ${PHS} phasor_cpp_compiler
        COMMENT "Building PHS header ${HEADER}"
    )

    configure_file(
        "${CMAKE_SOURCE_DIR}/src/App/utils/coreutils/coreutils_main.in.cpp"
        "${GEN_CPP}"
        @ONLY
    )

    add_executable(${tgt}
        ${GEN_CPP}
        src/App/utils/coreutils/${tgt}.rc
        ${HEADER}
    )

    target_include_directories(${tgt} PRIVATE ${BIN_DIR})
    set_target_properties(${tgt} PROPERTIES OUTPUT_NAME "${tgt}")
    target_link_libraries(${tgt} phasor_native_runtime)

    install(TARGETS ${tgt}
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    FRAMEWORK DESTINATION "frameworks"
    )
endforeach()