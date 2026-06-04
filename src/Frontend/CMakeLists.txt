add_library(PhasorFrontend STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Frontend.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Frontend.hpp
)
target_link_libraries(PhasorFrontend PUBLIC phasor_language PhasorCodegen PhasorRuntime)