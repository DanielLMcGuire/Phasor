set(TARGETS cat cp echo ls mv rm touch)

foreach(tgt ${TARGETS})
    set(PHS "${CMAKE_SOURCE_DIR}/src/utils/coreutils/${tgt}.phs")
    set(HEADER "${CMAKE_CURRENT_BINARY_DIR}/${tgt}.hpp")
    set(GEN_CPP "${CMAKE_CURRENT_BINARY_DIR}/${tgt}.cpp")
    set(PATH_COUT "src/Executable/utils/coreutils/${tgt}.hpp")

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    add_custom_command(
        OUTPUT ${HEADER}
        COMMAND $<TARGET_FILE:phasor_cxx_transpiler> ${PHS} -o ${HEADER} -H --nologo
        DEPENDS ${PHS} phasor_cxx_transpiler
        COMMENT "Building PHS object ${PATH_COUT}"
    )

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/coreutils_main.in.cpp"
        "${GEN_CPP}"
        @ONLY
    )

    add_executable(${tgt}
        ${GEN_CPP}
        ${CMAKE_CURRENT_SOURCE_DIR}/${tgt}.rc
        ${HEADER}
        ${PHS}
    )
    target_include_directories(${tgt} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
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