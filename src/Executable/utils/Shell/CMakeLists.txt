add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/utils/Shell/Shell.hpp
    COMMAND $<TARGET_FILE:phasor_cpp_compiler> ${CMAKE_SOURCE_DIR}/src/utils/Shell/Shell.phs -o ${CMAKE_CURRENT_BINARY_DIR}/src/utils/Shell/Shell.hpp -H --nologo
    DEPENDS ${CMAKE_SOURCE_DIR}/src/utils/Shell/Shell.phs phasor_cpp_compiler
    COMMENT "Building PHS bytecode src/Library/utils/Shell/Shell.hpp"
)
add_executable(phasor_shell
    Shell.rc
    shell_main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/utils/Shell/Shell.hpp
    ${CMAKE_SOURCE_DIR}/src/utils/Shell/Shell.phs
)
target_include_directories(phasor_shell PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src/utils/Shell)
target_link_libraries(phasor_shell phasor_native_runtime)
set_target_properties(phasor_shell PROPERTIES OUTPUT_NAME phash)