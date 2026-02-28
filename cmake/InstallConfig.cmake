if(WIN32)
    install(TARGETS
        phasor_main
        phasor_compiler
        phasor_repl
        phasor_interpreter
        phasor_cxx_transpiler
        phasor_lsp
    
        pulsar_main
        pulsar_compiler

        phasor_disasm
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
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
        phasor_compiler
        phasor_repl
        phasor_interpreter
        phasor_cxx_transpiler
        phasor_lsp
    
        pulsar_main
        pulsar_compiler

        phasor_disasm
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
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
        phasor_compiler
        phasor_repl
        phasor_interpreter
        phasor_cxx_transpiler
        phasor_lsp
    
        pulsar_main
        pulsar_compiler

        phasor_disasm
        phasor_runtime_exe
        phasor_native_runtime
        phasor_native_runtime_static
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