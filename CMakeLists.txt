cmake_minimum_required(VERSION 3.10)
project(Phasor VERSION 2.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(include src)

option(ASSEMBLY "Enable assembly for core code" ON)

if(MSVC)
    enable_language(RC)
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
    if(ASSEMBLY)
        enable_language(ASM_MASM)
    endif()
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
    if(ASSEMBLY)
        enable_language(ASM)
    endif()
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
    if(ASSEMBLY)
        enable_language(ASM)
    endif()
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

set(RUNTIME_SOURCES ${CMAKE_SOURCE_DIR}/src/Runtime/VM/VM.cpp  ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/ffi.cpp)
if(ASSEMBLY)
    list(APPEND RUNTIME_SOURCES ${ARITHMETIC_ASM} ${LOGICAL_ASM} ${IO_ASM})
else()
    list(APPEND RUNTIME_SOURCES
        ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/arithmetic/crt.c
        ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/logical/crt.c
        ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/IO/crt.c
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

add_library(PhasorStdlib STATIC ${STDLIB_SOURCES} ${STDLIB_HEADERS})

add_library(PhasorRuntime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS})
target_link_libraries(PhasorRuntime PUBLIC PhasorStdlib)

set(INTERPRETER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Backend/Lexer/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/Backend/Parser/Parser.cpp
)
set(INTERPRETER_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Backend/Lexer/Lexer.hpp
    ${CMAKE_SOURCE_DIR}/src/Backend/Parser/Parser.hpp
)
add_library(PhasorInterpreter STATIC ${INTERPRETER_SOURCES} ${INTERPRETER_HEADERS})

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
add_library(PhasorCodegen STATIC ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})

set(FRONTEND_SOURCES ${CMAKE_SOURCE_DIR}/src/Frontend/Frontend.cpp)
set(FRONTEND_HEADERS ${CMAKE_SOURCE_DIR}/src/Frontend/Frontend.hpp)
add_library(PhasorFrontend STATIC ${FRONTEND_SOURCES} ${FRONTEND_HEADERS})
target_link_libraries(PhasorFrontend PUBLIC PhasorCodegen PhasorRuntime)

add_executable(phasor_repl
    ${CMAKE_SOURCE_DIR}/src/App/Repl/Repl.rc
    ${CMAKE_SOURCE_DIR}/src/App/Repl/Repl_main.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Repl/Repl.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Repl/Repl.hpp
)
target_link_libraries(phasor_repl PhasorRuntime PhasorInterpreter PhasorFrontend)
set_target_properties(phasor_repl PROPERTIES OUTPUT_NAME phasorrepl)

add_executable(phasor_compiler
    ${CMAKE_SOURCE_DIR}/src/App/Compiler/Compiler.rc
    ${CMAKE_SOURCE_DIR}/src/App/Compiler/Compiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Compiler/Compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Compiler/Compiler.hpp
)
target_link_libraries(phasor_compiler PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_compiler PROPERTIES OUTPUT_NAME phasorcompiler)

add_executable(phasor_cpp_compiler
    ${CMAKE_SOURCE_DIR}/src/App/Compiler/CppCompiler.rc
    ${CMAKE_SOURCE_DIR}/src/App/Compiler/CppCompiler_main.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Compiler/CppCompiler.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Compiler/CppCompiler.hpp
)
target_link_libraries(phasor_cpp_compiler PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_cpp_compiler PROPERTIES OUTPUT_NAME phasornative)

add_executable(phasor_runtime_exe
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/BinaryRuntime.rc
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/BinaryRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/BinaryRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/BinaryRuntime.hpp
)
target_link_libraries(phasor_runtime_exe PhasorRuntime PhasorCodegen PhasorFrontend)
set_target_properties(phasor_runtime_exe PROPERTIES OUTPUT_NAME phasorvm)

add_library(phasor_native_runtime SHARED
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/NativeRuntimeDll.rc
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.hpp
)
target_link_libraries(phasor_native_runtime PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)

add_library(phasor_native_runtime_static STATIC
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.hpp
)
target_link_libraries(phasor_native_runtime_static PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)

add_executable(phasor_interpreter
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/ScriptingRuntime.rc
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/ScriptingRuntime_main.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/ScriptingRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/ScriptingRuntime.hpp
)
target_link_libraries(phasor_interpreter PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_interpreter PROPERTIES OUTPUT_NAME phasorjit)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/src/CLI/utils/Shell.hpp
    COMMAND $<TARGET_FILE:phasor_cpp_compiler> ${CMAKE_SOURCE_DIR}/src/CLI/utils/Shell.phs -o ${CMAKE_BINARY_DIR}/src/CLI/utils/Shell.hpp -H --nologo
    DEPENDS ${CMAKE_SOURCE_DIR}/src/CLI/utils/Shell.phs phasor_cpp_compiler
    COMMENT "Building PHS object src/CLI/utils/Shell.phs"
)
add_executable(phasor_shell
    ${CMAKE_SOURCE_DIR}/src/App/utils/Shell.rc
    ${CMAKE_SOURCE_DIR}/src/App/utils/shell_main.cpp
    ${CMAKE_BINARY_DIR}/src/CLI/utils/Shell.hpp
	${CMAKE_SOURCE_DIR}/src/CLI/utils/Shell.phs
)
target_include_directories(phasor_shell PRIVATE ${CMAKE_BINARY_DIR}/src/CLI/utils)
target_link_libraries(phasor_shell phasor_native_runtime)
set_target_properties(phasor_shell PROPERTIES OUTPUT_NAME phash)

add_executable(phasor_winapi_gen
    ${CMAKE_SOURCE_DIR}/src/tools/windows/bindings.cpp
)
set_target_properties(phasor_winapi_gen PROPERTIES OUTPUT_NAME winapi-gen-phs)

install(TARGETS
    phasor_compiler phasor_cpp_compiler phasor_runtime_exe
    phasor_native_runtime phasor_native_runtime_static
    phasor_repl phasor_interpreter phasor_shell
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FRAMEWORK DESTINATION frameworks
)

set(TARGETS cat cp echo ls mv rm touch)
set(BIN_DIR "${CMAKE_BINARY_DIR}/src/CLI/utils/coreutils")

foreach(tgt ${TARGETS})
    set(PHS "${CMAKE_SOURCE_DIR}/src/CLI/utils/coreutils/${tgt}.phs")
    set(HEADER "${BIN_DIR}/${tgt}.hpp")
    set(GEN_CPP "${BIN_DIR}/${tgt}.cpp")

    file(MAKE_DIRECTORY ${BIN_DIR})

    add_custom_command(
        OUTPUT ${HEADER}
        COMMAND $<TARGET_FILE:phasor_cpp_compiler> ${PHS} -o ${HEADER} -H --nologo
        DEPENDS ${PHS} phasor_cpp_compiler
        COMMENT "Building PHS object ${PHS}"
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
		${PHS}
    )
    target_include_directories(${tgt} PRIVATE ${BIN_DIR})
    set_target_properties(${tgt} PROPERTIES OUTPUT_NAME "${tgt}-phs")
    target_link_libraries(${tgt} phasor_native_runtime)

    install(TARGETS ${tgt}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        FRAMEWORK DESTINATION frameworks
    )
endforeach()
