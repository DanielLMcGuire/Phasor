add_library(PhasorFrontend STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Frontend.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Frontend.hpp
)
target_link_libraries(PhasorFrontend PUBLIC PhasorCodegen PhasorRuntime)

add_library(PulsarFrontend STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Frontend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Frontend.hpp
)
target_link_libraries(PulsarFrontend PUBLIC PhasorCodegen PhasorRuntime)