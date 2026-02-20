set(PHASOR_LANGUAGE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Lexer/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Parser/Parser.cpp
)

set(PHASOR_LANGUAGE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Lexer/Lexer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Parser/Parser.hpp
)

set(PULSAR_LANGUAGE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Lexer/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Parser/Parser.cpp
)

set(PULSAR_LANGUAGE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Lexer/Lexer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/Parser/Parser.cpp
)

add_library(phasor_language STATIC ${PHASOR_LANGUAGE_SOURCES} ${PHASOR_LANGUAGE_HEADERS})

add_library(pulsar_language STATIC ${PULSAR_LANGUAGE_SOURCES} ${PULSAR_LANGUAGE_HEADERS})