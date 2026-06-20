if(IS_XBOX OR EMBEDDED)
    install(TARGETS
        phasor_main
        phasor_native_runtime
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    install(FILES
        ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
        ${CMAKE_SOURCE_DIR}/include/PhasorRT.h
        DESTINATION include
    )
elseif(WIN32)
    set(NON_STATIC_TARGETS
        phasor_main
        phasorw_main
        phasor_help
        phasor_compiler
        phasor_cxx_transpiler
        phasor_lsp

        phasor_asm
        phasor_disasm
        phasor_runtime_exe
        phasor_native_runtime
    )
    install(TARGETS 
        ${NON_STATIC_TARGETS}
        phasor_native_runtime_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    foreach(TARGET ${NON_STATIC_TARGETS})
        install(
            FILES "$<$<NOT:$<CONFIG:Release>>:$<TARGET_PDB_FILE:${TARGET}>>"
            DESTINATION bin
            OPTIONAL
        )
    endforeach()
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/
        DESTINATION include
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/docs/man/
        DESTINATION man
        PATTERN "*.sh" EXCLUDE
        PATTERN "*.py" EXCLUDE
        PATTERN "*.1" EXCLUDE
        PATTERN "*.3" EXCLUDE
        PATTERN "*.5" EXCLUDE
        PATTERN "*.7" EXCLUDE
        PATTERN "*.md5" EXCLUDE
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/
        DESTINATION "${INCLUDE_INSTALL_DIR}"
    )
    install(TARGETS phasor_winapi_bindings RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES "$<$<NOT:$<CONFIG:Release>>:$<TARGET_PDB_FILE:phasor_winapi_bindings>>" DESTINATION ${PLUGIN_INSTALL_DIR})
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/src/Bindings/win32/phs/
        DESTINATION "${INCLUDE_INSTALL_DIR}"
    )
    if(MSYS)
        install(TARGETS phasor_posix_bindings
            LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
            RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
        )
    endif()
    if(PHASOR_SDL2)
        install(TARGETS phasor_sdl2_bindings
            LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
            RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
        )
        install(FILES "$<$<NOT:$<CONFIG:Release>>:$<TARGET_PDB_FILE:phasor_sdl2_bindings>>" DESTINATION ${PLUGIN_INSTALL_DIR})

        install(DIRECTORY
            ${CMAKE_SOURCE_DIR}/src/Bindings/sdl2/phs/
            DESTINATION "${INCLUDE_INSTALL_DIR}"
        )
    endif()
elseif(APPLE)
    install(TARGETS
        phasor_main
        phasor_help
        phasor_compiler
        phasor_cxx_transpiler
        phasor_lsp

        phasor_asm
        phasor_disasm
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
        RUNTIME DESTINATION usr/local/bin
        LIBRARY DESTINATION usr/local/lib
        ARCHIVE DESTINATION usr/local/lib
        FRAMEWORK DESTINATION frameworks
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/
        DESTINATION usr/local/include
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/docs/man/
        DESTINATION usr/local/share/man
        PATTERN "*.sh" EXCLUDE
        PATTERN "*.py" EXCLUDE
        PATTERN "*.pdf" EXCLUDE
        PATTERN "*.md5" EXCLUDE
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/
        DESTINATION opt/phasor
    )
    install(TARGETS phasor_posix_bindings
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
    )
    if(PHASOR_SDL2)
        install(TARGETS phasor_sdl2_bindings RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR})

        install(DIRECTORY
            ${CMAKE_SOURCE_DIR}/src/Bindings/sdl2/phs/
            DESTINATION "${INCLUDE_INSTALL_DIR}"
        )
    endif()
else()
    install(TARGETS
        phasor_main
        phasor_help
        phasor_compiler
        phasor_cxx_transpiler
        phasor_lsp

        phasor_asm
        phasor_disasm
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
        RUNTIME DESTINATION usr/bin
        LIBRARY DESTINATION usr/lib
        ARCHIVE DESTINATION usr/lib
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/
        DESTINATION usr/include
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/docs/man/
        DESTINATION usr/share/man/
        PATTERN "*.sh" EXCLUDE
        PATTERN "*.py" EXCLUDE
        PATTERN "*.pdf" EXCLUDE
        PATTERN "*.md5" EXCLUDE
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/
        DESTINATION "${INCLUDE_INSTALL_DIR}"
    )
    install(TARGETS phasor_posix_bindings
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
    )
    if(PHASOR_SDL2)
        install(TARGETS phasor_sdl2_bindings RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR})

        install(DIRECTORY
            ${CMAKE_SOURCE_DIR}/src/Bindings/sdl2/phs/
            DESTINATION "${INCLUDE_INSTALL_DIR}"
        )
    endif()
endif()