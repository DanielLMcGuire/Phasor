add_library(phasor_lsp_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/LSP.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/LSP.hpp
)

target_link_libraries(phasor_lsp_lib phasor_language PhasorCodegen)