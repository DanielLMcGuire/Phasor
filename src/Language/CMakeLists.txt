set(PHASOR_LANGUAGE_SOURCES
    Phasor/Lexer/Lexer.cpp
    Phasor/Parser/Parser.cpp
)

set(PHASOR_LANGUAGE_HEADERS
    Phasor/Lexer/Lexer.hpp
    Phasor/Parser/Parser.hpp
)

set(PULSAR_LANGUAGE_SOURCES
    Pulsar/Lexer/Lexer.cpp
    Pulsar/Parser/Parser.cpp
)

set(PULSAR_LANGUAGE_HEADERS
    Pulsar/Lexer/Lexer.hpp
    Pulsar/Parser/Parser.cpp
)

add_library(phasor_language STATIC ${PHASOR_LANGUAGE_SOURCES} ${PHASOR_LANGUAGE_HEADERS})

add_library(pulsar_language STATIC ${PULSAR_LANGUAGE_SOURCES} ${PULSAR_LANGUAGE_HEADERS})