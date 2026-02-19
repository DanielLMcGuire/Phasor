set(TARGETS cat cp echo ls mv rm touch)
set(BIN_RAW "src/utils/coreutils")
set(BIN_DIR "${CMAKE_BINARY_DIR}/${BIN_RAW}")

foreach(tgt ${TARGETS})
    set(PHS "${CMAKE_SOURCE_DIR}/src/utils/coreutils/${tgt}.phs")
    set(HEADER "${BIN_DIR}/${tgt}.hpp")
    set(GEN_CPP "${BIN_DIR}/${tgt}.cpp")
    set(PATH_COUT "${BIN_RAW}/${tgt}.hpp")

    file(MAKE_DIRECTORY ${BIN_DIR})

    add_custom_command(
        OUTPUT ${HEADER}
        COMMAND $<TARGET_FILE:phasor_cpp_compiler> ${PHS} -o ${HEADER} -H --nologo
        DEPENDS ${PHS} phasor_cpp_compiler
        COMMENT "Building PHS object ${PATH_COUT}"
    )

    configure_file(
        "${CMAKE_SOURCE_DIR}/src/Executable/utils/coreutils/coreutils_main.in.cpp"
        "${GEN_CPP}"
        @ONLY
    )

    add_executable(${tgt}
        ${GEN_CPP}
        ${CMAKE_SOURCE_DIR}/src/Executable/utils/coreutils/${tgt}.rc
        ${HEADER}
        ${PHS}
    )
    target_include_directories(${tgt} PRIVATE ${BIN_DIR})
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