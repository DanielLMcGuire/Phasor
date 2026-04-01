if (IS_SANDBOXED)
    set(STDLIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/StdLib.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/math.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/string.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/system.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/typeconv.cpp
    )
else()
    set(STDLIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/StdLib.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/file.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/file_properties.c
        ${CMAKE_CURRENT_SOURCE_DIR}/core/system.c
        ${CMAKE_CURRENT_SOURCE_DIR}/io.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/math.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/string.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/system.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/typeconv.cpp
    )
endif()
set(STDLIB_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/file_properties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/system.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StdLib.hpp
)

if(EMSCRIPTEN)
set(STDLIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/StdLib.cpp PARENT_SCOPE)
else()
set(STDLIB_SOURCES ${STDLIB_SOURCES} PARENT_SCOPE)
endif()
set(STDLIB_HEADERS ${STDLIB_HEADERS} PARENT_SCOPE)