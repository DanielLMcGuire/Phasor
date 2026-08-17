set(NATIVE_RUNTIME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/NativeRuntimeDll.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Main.cpp
)

if (WIN32)
    list(APPEND NATIVE_RUNTIME_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/NativeRuntime_com.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/NativeRuntime_com.hpp
    )
endif()

add_library(phasor_native_runtime SHARED
    ${NATIVE_RUNTIME_SOURCES}
)
target_link_libraries(phasor_native_runtime phasor_native_runtime_lib PhasorFrontend)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)

add_library(phasor_native_runtime_static STATIC
    ${NATIVE_RUNTIME_SOURCES}
)

target_link_libraries(phasor_native_runtime_static PRIVATE
    phasor_native_runtime_lib
    PhasorFrontend
)

set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)

if(PROJECT_IS_TOP_LEVEL)

set(BUILD_POSIX_BINDINGS OFF)

add_executable(phasor_main 
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor_main.c
)
target_link_libraries(phasor_main phasor_native_runtime)
set_target_properties(phasor_main PROPERTIES OUTPUT_NAME phasor)

if(WIN32)
    add_executable(phasorw_main
        ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor.rc
        ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor_main.c
    )
    target_link_libraries(phasorw_main phasor_native_runtime)
    set_target_properties(phasorw_main PROPERTIES OUTPUT_NAME phasorw)
    target_compile_definitions(phasorw_main PRIVATE PHS_WINDOWED)
    set_target_properties(phasorw_main PROPERTIES WIN32_EXECUTABLE TRUE)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(phasorw_main PRIVATE -mwindows)
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        target_link_options(phasorw_main PRIVATE /ENTRY:mainCRTStartup)
    else()
        # plain clang, MSVC ABI, lld-link via GNU-style driver
        target_link_options(phasorw_main PRIVATE -Xlinker -entry:mainCRTStartup)
    endif()
endif()

if(NOT IS_XBOX AND NOT EMBEDDED)

add_executable(phasor_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler_main.c
)
target_link_libraries(phasor_compiler phasor_native_runtime)
set_target_properties(phasor_compiler PROPERTIES OUTPUT_NAME phasorc)

add_executable(phasor_lsp
    ${CMAKE_CURRENT_SOURCE_DIR}/LSP/Phasor/LSP_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LSP/Phasor/LSP.rc
)
target_link_libraries(phasor_lsp phasor_lsp_lib)
set_target_properties(phasor_lsp PROPERTIES OUTPUT_NAME phasor-lsp)

add_executable(phasor_disasm
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Disassembler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Disassembler_main.cpp
)
target_link_libraries(phasor_disasm phasor_disasm_lib)
set_target_properties(phasor_disasm PROPERTIES OUTPUT_NAME phasordecomp)

add_executable(phasor_asm
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Assembler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Shared/Assembler_main.cpp
)
target_link_libraries(phasor_asm phasor_asm_lib)
set_target_properties(phasor_asm PROPERTIES OUTPUT_NAME phasorasm)

add_executable(phasor_c_transpiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/CCompiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/CCompiler_main.cpp
)
target_link_libraries(phasor_c_transpiler phasor_c_transpiler_lib)
set_target_properties(phasor_c_transpiler PROPERTIES OUTPUT_NAME phasorcc)

add_executable(phasor_runtime_exe
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/BinaryRuntime.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/BinaryRuntime_main.cpp
)
target_link_libraries(phasor_runtime_exe phasor_binary_runtime_lib)
set_target_properties(phasor_runtime_exe PROPERTIES OUTPUT_NAME phasorvm)

endif() # NOT IS_XBOX AND NOT EMBEDDED

if(NOT IS_SANDBOXED)
    if(PHASOR_SDL2)
        add_library(phasor_sdl2_bindings SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/sdl2/sdl2_main.cpp
        )
        target_link_libraries(phasor_sdl2_bindings SDL2::SDL2-static)
        set_target_properties(phasor_sdl2_bindings PROPERTIES OUTPUT_NAME sdl2_phs)
    endif()
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
        set_target_properties(phasor_winapi_bindings PROPERTIES OUTPUT_NAME winapi_phs)
        if (MSYS)
            set(BUILD_POSIX_BINDINGS ON)
        endif()
    else()
        if(APPLE)
            enable_language(OBJC)
            add_library(phasor_applescript_bindings SHARED
                ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/macOS/main.c
                ${CMAKE_SOURCE_DIR}/src/Bindings/macOS/AppleScript.m
                ${CMAKE_SOURCE_DIR}/src/Bindings/macOS/AppleScript.h
            )
            target_link_libraries(phasor_applescript_bindings
                "-framework Foundation"
            )
            set_target_properties(phasor_applescript_bindings PROPERTIES OUTPUT_NAME applescript_phs)
        endif()
        set(BUILD_POSIX_BINDINGS ON)
    endif()
endif() # NOT IS_SANDBOXED

if(NOT IS_XBOX AND NOT EMBEDDED)
    add_subdirectory(Help)
endif()

if(BUILD_POSIX_BINDINGS)
    add_library(phasor_posix_bindings SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/posix/main.c
    )
    set_target_properties(phasor_posix_bindings PROPERTIES OUTPUT_NAME posix_phs)
endif()

endif() # PROJECT_IS_TOP_LEVEL