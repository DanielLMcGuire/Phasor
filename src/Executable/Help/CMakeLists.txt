add_executable(phasor_help
    phasor_help.rc
    phasor_help_bootstrap.cpp
)
phasor_add_transpiled_header(
    TARGET phasor_help
    NAME main
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/phasor_help_main.phs"
)

if(PHS_DEBUG_ENABLED)
    phasor_add_transpiled_defines(
        TARGET phasor_help
        NAME main
        DEFINES PHS_DEBUG
    )
endif()
target_link_libraries(phasor_help phasor_native_runtime)
target_precompile_headers(phasor_help PRIVATE ${phasor_help_main_PHS_HEADER})
set_target_properties(phasor_help PROPERTIES OUTPUT_NAME phasor-help)