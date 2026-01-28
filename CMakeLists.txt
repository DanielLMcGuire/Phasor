cmake_minimum_required(VERSION 3.10)
project(Phasor VERSION 2.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
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

if(MSVC)
    enable_language(RC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox /DNOMINMAX /EHsc /fp:precise /arch:AVX2 /Oi /Ot /GL /Gy /Qspectre- /W3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Ox /fp:precise /arch:AVX2 /Oi /Ot /GL /Gy /Qspectre-")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /OPT:REF /OPT:ICF")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG /OPT:REF /OPT:ICF")
    
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /DNOMINMAX /EHsc /fp:strict")
    set(CMAKE_C_FLAGS_DEBUG "/Od /Zi /fp:strict")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -flto -ffast-math -funroll-loops -fomit-frame-pointer -frtti -fexceptions -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -flto -ffast-math -funroll-loops -fomit-frame-pointer -fexceptions")
    
    if(APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-dead_strip")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-dead_strip")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
    endif()
    
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer -frtti -fexceptions")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer -fexceptions")
endif()


if(MSVC)
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
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/VM.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/ffi.cpp
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

set(INTERPRETER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Backend/Lexer/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/Backend/Parser/Parser.cpp
)

set(INTERPRETER_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Backend/Lexer/Lexer.hpp
    ${CMAKE_SOURCE_DIR}/src/Backend/Parser/Parser.hpp
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

add_library(PhasorStdlib STATIC ${STDLIB_SOURCES} ${STDLIB_HEADERS})

add_library(PhasorRuntime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS})
target_link_libraries(PhasorRuntime PUBLIC PhasorStdlib)

add_library(PhasorInterpreter STATIC ${INTERPRETER_SOURCES} ${INTERPRETER_HEADERS})

add_library(PhasorCodegen STATIC ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

add_library(PhasorFrontend STATIC 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Frontend.cpp 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Frontend.hpp 
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)
target_link_libraries(PhasorFrontend PUBLIC PhasorCodegen PhasorRuntime)

add_library(phasor_native_runtime SHARED
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/NativeRuntimeDll.rc
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)

add_library(phasor_native_runtime_static STATIC
    ${CMAKE_SOURCE_DIR}/src/App/Runtime/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/CLI/Runtime/NativeRuntime.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime_static PhasorRuntime PhasorInterpreter PhasorCodegen PhasorFrontend)
set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)


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

if(WIN32)
    add_executable(phasor_winapi_gen
        ${CMAKE_SOURCE_DIR}/src/Bindings/win32/binding_generator.cpp
    )
    set_target_properties(phasor_winapi_gen PROPERTIES OUTPUT_NAME winapi-gen-phs)

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/phasor_winapi_bindings.cpp
        COMMAND $<TARGET_FILE:phasor_winapi_gen> ${CMAKE_SOURCE_DIR}/scripts/winapi.h -o ${CMAKE_BINARY_DIR}/phasor_winapi_bindings.cpp
        DEPENDS ${CMAKE_SOURCE_DIR}/scripts/winapi.h phasor_winapi_gen
        COMMENT "Building scripts/winapi.h"
    )

    add_library(phasor_winapi_bindings SHARED
        ${CMAKE_BINARY_DIR}/phasor_winapi_bindings.cpp
        ${CMAKE_SOURCE_DIR}/src/Bindings/win32/handle.hpp
        ${CMAKE_SOURCE_DIR}/scripts/winapi.h
    )
    set_target_properties(phasor_winapi_bindings PROPERTIES 
        OUTPUT_NAME winapi_phs
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    )

    install(TARGETS phasor_winapi_bindings RUNTIME DESTINATION PLUGIN_INSTALL_DIR)
else()
    add_library(phasor_posix_bindings SHARED
        ${CMAKE_SOURCE_DIR}/src/Bindings/posix_bindings.c
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


add_executable(clocf ${CMAKE_SOURCE_DIR}/src/tools/extra/clocf/clocf.c)
set_target_properties(clocf PROPERTIES EXCLUDE_FROM_ALL TRUE)

if(WIN32)
    install(TARGETS
        phasor_compiler
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
elseif(APPLE)
    install(TARGETS
    phasor_compiler
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
else()
    install(TARGETS
        phasor_compiler
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
endif()

install(FILES
    ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
    DESTINATION include
)

if(WIN32)
install(DIRECTORY "${CMAKE_BINARY_DIR}/" 
        DESTINATION bin 
        FILES_MATCHING 
        PATTERN "*.pdb" 
        PATTERN "CMakeFiles" EXCLUDE)
endif()
