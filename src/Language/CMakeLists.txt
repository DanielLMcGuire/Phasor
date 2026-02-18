set(PHASOR_LANGUAGE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Lexer/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Parser/Parser.cpp
)

set(PHASOR_LANGUAGE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Lexer/Lexer.hpp
    ${CMAKE_SOURCE_DIR}/src/Language/Phasor/Parser/Parser.hpp
)

set(PULSAR_LANGUAGE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Lexer/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Parser/Parser.cpp
)

set(PULSAR_LANGUAGE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Lexer/Lexer.hpp
    ${CMAKE_SOURCE_DIR}/src/Language/Pulsar/Parser/Parser.cpp
)

add_library(phasor_language STATIC ${PHASOR_LANGUAGE_SOURCES} ${PHASOR_LANGUAGE_HEADERS})

add_library(pulsar_language STATIC ${PULSAR_LANGUAGE_SOURCES} ${PULSAR_LANGUAGE_HEADERS})