set(PHASOR_LANGUAGE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Lexer/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Parser/Parser.cpp
)

set(PHASOR_LANGUAGE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Lexer/Lexer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/Parser/Parser.hpp
)

add_library(phasor_language STATIC ${PHASOR_LANGUAGE_SOURCES} ${PHASOR_LANGUAGE_HEADERS})