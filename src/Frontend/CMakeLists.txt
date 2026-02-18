add_library(PhasorFrontend STATIC 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Phasor/Frontend.cpp 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Phasor/Frontend.hpp
)
target_link_libraries(PhasorFrontend PUBLIC PhasorCodegen PhasorRuntime)

add_library(PulsarFrontend STATIC 
    ${CMAKE_SOURCE_DIR}/src/Frontend/Pulsar/Frontend.cpp
    ${CMAKE_SOURCE_DIR}/src/Frontend/Pulsar/Frontend.hpp
)
target_link_libraries(PulsarFrontend PUBLIC PhasorCodegen PhasorRuntime)