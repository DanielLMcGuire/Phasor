add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/utils/pmake/pmake.hpp
    COMMAND $<TARGET_FILE:phasor_cpp_compiler> ${CMAKE_SOURCE_DIR}/src/utils/pmake/pmake.phs -o ${CMAKE_CURRENT_BINARY_DIR}/src/utils/pmake/pmake.hpp -H --nologo
    DEPENDS ${CMAKE_SOURCE_DIR}/src/utils/pmake/pmake.phs phasor_cpp_compiler
    COMMENT "Building PHS bytecode src/utils/pmake/pmake.hpp"
)
add_executable(pmake
    pmake.rc
    pmake_main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/utils/pmake/pmake.hpp
    ${CMAKE_SOURCE_DIR}/src/utils/pmake/pmake.phs
)
target_include_directories(pmake PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src/utils/pmake)
target_link_libraries(pmake phasor_native_runtime_static)
set_target_properties(pmake PROPERTIES EXCLUDE_FROM_ALL TRUE)