set(NATIVE_RUNTIME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/NativeRuntimeDll.rc
)

if (WIN32)
    list(APPEND NATIVE_RUNTIME_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/NativeRuntime_com.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/NativeRuntime_com.hpp
    )
endif()

add_library(phasor_native_runtime SHARED
    ${NATIVE_RUNTIME_SOURCES}
)
target_link_libraries(phasor_native_runtime phasor_native_runtime_lib PhasorFrontend)
target_compile_definitions(phasor_native_runtime PRIVATE _SHARED)
set_target_properties(phasor_native_runtime PROPERTIES OUTPUT_NAME phasorrt)

add_library(phasor_native_runtime_static STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Shared/NativeRuntime_library.cpp
    ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
)
target_link_libraries(phasor_native_runtime_static phasor_native_runtime_lib PhasorFrontend)
set_target_properties(phasor_native_runtime_static PROPERTIES OUTPUT_NAME phasorstaticrt)

if(MSVC)
    add_custom_command(TARGET phasor_native_runtime_static POST_BUILD
        COMMAND "${CMAKE_AR}" /NOLOGO /OUT:$<TARGET_FILE:phasor_native_runtime_static>
            $<TARGET_FILE:phasor_native_runtime_static>
            $<TARGET_FILE:phasor_native_runtime_lib>
            $<TARGET_FILE:PhasorFrontend>
            $<TARGET_FILE:phasor_language>
            $<TARGET_FILE:PhasorRuntime>
            $<TARGET_FILE:PhasorCodegen>
        COMMENT "Merging dependencies into phasorstaticrt.lib (MSVC)..."
    )
elseif(APPLE)
    add_custom_command(TARGET phasor_native_runtime_static POST_BUILD
        COMMAND libtool -static -o $<TARGET_FILE:phasor_native_runtime_static>
            $<TARGET_FILE:phasor_native_runtime_static>
            $<TARGET_FILE:phasor_native_runtime_lib>
            $<TARGET_FILE:PhasorFrontend>
            $<TARGET_FILE:phasor_language>
            $<TARGET_FILE:PhasorRuntime>
            $<TARGET_FILE:PhasorCodegen>
        COMMENT "Merging dependencies into libphasorstaticrt.a (macOS)..."
    )
else()
    set(mri_file "${CMAKE_CURRENT_BINARY_DIR}/phasorstaticrt.mri")
    
    set(mri_content "create $<TARGET_FILE:phasor_native_runtime_static>\n")
    set(mri_content "${mri_content}addlib $<TARGET_FILE:phasor_native_runtime_static>\n")
    set(mri_content "${mri_content}addlib $<TARGET_FILE:phasor_native_runtime_lib>\n")
    set(mri_content "${mri_content}addlib $<TARGET_FILE:PhasorFrontend>\n")
    set(mri_content "${mri_content}addlib $<TARGET_FILE:phasor_language>\n")
    set(mri_content "${mri_content}addlib $<TARGET_FILE:PhasorRuntime>\n")
    set(mri_content "${mri_content}addlib $<TARGET_FILE:PhasorCodegen>\n")
    set(mri_content "${mri_content}save\nend\n")

    file(GENERATE 
        OUTPUT "${mri_file}" 
        CONTENT "${mri_content}"
    )

    add_custom_command(TARGET phasor_native_runtime_static POST_BUILD
        COMMAND "${CMAKE_AR}" -M < "${mri_file}"
        COMMENT "Merging dependencies into libphasorstaticrt.a (GNU/LLVM)..."
    )
endif()

if(PROJECT_IS_TOP_LEVEL)

add_executable(phasor_main 
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Main/Phasor/Phasor_main.cpp
)
target_link_libraries(phasor_main phasor_scripting_runtime_lib phasor_binary_runtime_lib PhasorFrontend)
set_target_properties(phasor_main PROPERTIES OUTPUT_NAME phasor)

if(NOT IS_XBOX AND NOT EMBEDDED)

add_executable(phasor_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Phasor/Compiler_main.cpp
)
target_link_libraries(phasor_compiler phasor_compiler_lib)
set_target_properties(phasor_compiler PROPERTIES OUTPUT_NAME phasorcompiler)

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

endif() # NOT IS_XBOX AND NOT EMBEDDED

if(NOT IS_SANDBOXED)
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
        install(TARGETS phasor_winapi_bindings RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR})
        install(FILES "$<$<NOT:$<CONFIG:Release>>:$<TARGET_PDB_FILE:phasor_winapi_bindings>>" DESTINATION ${PLUGIN_INSTALL_DIR})
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
        add_library(phasor_posix_bindings SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/posix/main.c
        )
        set_target_properties(phasor_posix_bindings PROPERTIES OUTPUT_NAME posix_phs)
        install(TARGETS phasor_posix_bindings
            LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
            RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
        )
    endif()
endif() # NOT IS_SANDBOXED

if(NOT IS_XBOX AND NOT EMBEDDED)
    add_subdirectory(Help)
endif()

endif() # PROJECT_IS_TOP_LEVEL