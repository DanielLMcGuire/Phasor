add_library(PhasorFrontend STATIC 
    Phasor/Frontend.cpp 
    Phasor/Frontend.hpp
)
target_link_libraries(PhasorFrontend PUBLIC PhasorCodegen PhasorRuntime)

add_library(PulsarFrontend STATIC 
    Pulsar/Frontend.cpp
    Pulsar/Frontend.hpp
)
target_link_libraries(PulsarFrontend PUBLIC PhasorCodegen PhasorRuntime)