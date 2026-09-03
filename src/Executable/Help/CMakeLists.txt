add_executable(phasor_help
    phasor_help.rc
    ../Runtime/stub_bootstrap.c
)
phasor_add_transpiled_header(
    TARGET phasor_help
    NAME main
    SOURCE "${CMAKE_SOURCE_DIR}/src/modules/help/main.phs"
)

if(PHS_DEBUG_ENABLED)
    phasor_add_transpiled_defines(
        TARGET phasor_help
        NAME main
        DEFINES PHS_DEBUG
    )
endif()
if(PHS_TRACE_ENABLED)
    phasor_add_transpiled_defines(
        TARGET phasor_help
        NAME main
        DEFINES PHS_TRACE
    )
endif()
target_link_libraries(phasor_help phasor_native_runtime)
target_precompile_headers(phasor_help PRIVATE ${phasor_help_main_PHS_HEADER})
set_target_properties(phasor_help PROPERTIES OUTPUT_NAME phasor-help)