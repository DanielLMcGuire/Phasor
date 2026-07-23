file(GLOB_RECURSE _phasor_stdlib_headers CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include/*"
)
file(GLOB_RECURSE _phasor_stdlib_sources CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/src/*.phs"
)
set_property(GLOBAL PROPERTY PHASOR_STDLIB_HEADERS "${_phasor_stdlib_headers}")
set_property(GLOBAL PROPERTY PHASOR_STDLIB_SOURCES "${_phasor_stdlib_sources}")

function(phasor_add_transpiled_defines)
    set(oneValueArgs TARGET NAME)
    set(multiValueArgs DEFINES)
    cmake_parse_arguments(PHS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT PHS_TARGET)
        message(FATAL_ERROR "phasor_add_transpiled_defines: TARGET is required")
    endif()
    if(NOT TARGET ${PHS_TARGET})
        message(FATAL_ERROR "phasor_add_transpiled_defines: '${PHS_TARGET}' is not a target yet")
    endif()
    if(NOT PHS_NAME)
        message(FATAL_ERROR "phasor_add_transpiled_defines: NAME is required")
    endif()
    if(NOT PHS_DEFINES)
        return()
    endif()

    set_property(TARGET ${PHS_TARGET} APPEND PROPERTY
        PHASOR_DEFINES_${PHS_NAME} ${PHS_DEFINES})
endfunction()

function(phasor_add_transpiled_header)
    set(oneValueArgs TARGET NAME SOURCE OUTPUT_NAME)
    cmake_parse_arguments(PHS "" "${oneValueArgs}" "" ${ARGN})

    if(NOT PHS_TARGET)
        message(FATAL_ERROR "phasor_add_transpiled_header: TARGET is required")
    endif()
    if(NOT TARGET ${PHS_TARGET})
        message(FATAL_ERROR "phasor_add_transpiled_header: '${PHS_TARGET}' is not a target yet")
    endif()
    if(NOT PHS_SOURCE)
        message(FATAL_ERROR "phasor_add_transpiled_header: SOURCE is required")
    endif()
    if(NOT PHS_NAME)
        get_filename_component(PHS_NAME "${PHS_SOURCE}" NAME_WE)
    endif()
    if(NOT PHS_OUTPUT_NAME)
        set(PHS_OUTPUT_NAME "${PHS_NAME}.hpp")
    endif()

    get_property(_stdlib_headers GLOBAL PROPERTY PHASOR_STDLIB_HEADERS)
    get_property(_stdlib_sources GLOBAL PROPERTY PHASOR_STDLIB_SOURCES)

    set(_output "${CMAKE_CURRENT_BINARY_DIR}/${PHS_OUTPUT_NAME}")

    set(_defines_genex "$<TARGET_PROPERTY:${PHS_TARGET},PHASOR_DEFINES_${PHS_NAME}>")
    set(_defines_flag  "$<$<BOOL:${_defines_genex}>:-D>")
    set(_defines_value "$<$<BOOL:${_defines_genex}>:$<JOIN:${_defines_genex},$<COMMA>>>")

    add_custom_command(
        OUTPUT ${_output}
        COMMAND $<TARGET_FILE:phasor_cxx_transpiler>
                "${PHS_SOURCE}"
                -o "${_output}"
                -H
                -I "${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/phs/include"
                ${_defines_flag} ${_defines_value}
        DEPENDS
            phasor_cxx_transpiler
            "${PHS_SOURCE}"
            ${_stdlib_headers}
            ${_stdlib_sources}
        COMMENT "Building PHS bytecode ${PHS_OUTPUT_NAME}"
    )

    set(_helper_target "${PHS_TARGET}_${PHS_NAME}_phs_header")
    add_custom_target(${_helper_target} DEPENDS ${_output})
    add_dependencies(${PHS_TARGET} ${_helper_target})

    target_sources(${PHS_TARGET} PRIVATE "${PHS_SOURCE}")
    set_source_files_properties("${PHS_SOURCE}" PROPERTIES HEADER_FILE_ONLY TRUE)

    set(${PHS_TARGET}_${PHS_NAME}_PHS_HEADER "${_output}" PARENT_SCOPE)
endfunction()