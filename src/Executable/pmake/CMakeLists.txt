add_executable(pmake
    pmake.rc
    ../Runtime/stub_bootstrap.c
)
phasor_add_transpiled_header(
    TARGET pmake
    NAME main
    SOURCE "${CMAKE_SOURCE_DIR}/src/modules/pmake/main.phs"
)

if(PHS_DEBUG_ENABLED)
    phasor_add_transpiled_defines(
        TARGET pmake
        NAME main
        DEFINES PHS_DEBUG
    )
endif()
target_link_libraries(pmake phasor_native_runtime)
target_precompile_headers(pmake PRIVATE ${pmake_main_PHS_HEADER})
set_target_properties(pmake PROPERTIES OUTPUT_NAME pmake)