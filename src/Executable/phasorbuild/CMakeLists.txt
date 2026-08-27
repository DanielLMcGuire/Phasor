add_executable(phasor_build
    phasor_build.rc
    ../Runtime/stub_bootstrap.c
)
phasor_add_transpiled_header(
    TARGET phasor_build
    NAME main
    SOURCE "${CMAKE_SOURCE_DIR}/src/modules/phasorbuild/main.phs"
)

if(PHS_DEBUG_ENABLED)
    phasor_add_transpiled_defines(
        TARGET phasor_build
        NAME main
        DEFINES PHS_DEBUG
    )
endif()
target_link_libraries(phasor_build phasor_native_runtime)
target_precompile_headers(phasor_build PRIVATE ${phasor_build_main_PHS_HEADER})
set_target_properties(phasor_build PROPERTIES OUTPUT_NAME phasorbuild)