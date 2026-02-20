set(RUNTIME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/Native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/Operations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/Register.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/Stack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/Utility.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/Variables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FFI/ffi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FFI/api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/core/IO/IO.c
)

if(ASSEMBLY)
    list(APPEND RUNTIME_SOURCES ${ARITHMETIC_ASM} ${LOGICAL_ASM})
else()
    list(APPEND RUNTIME_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/VM/core/arithmetic/crt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/VM/core/logical/crt.c
    )
endif()

set(RUNTIME_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Value.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/VM.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/core/core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/core/arithmetic/arithmetic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/core/logical/logical.h
    ${CMAKE_CURRENT_SOURCE_DIR}/VM/core/IO/IO.h
    ${CMAKE_SOURCE_DIR}/src/sscanf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FFI/ffi.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
    ${CMAKE_SOURCE_DIR}/src/AST/AST.hpp
    ${CMAKE_BINARY_DIR}/include/version.h
)

set(STDLIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/core/file_properties.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/core/system.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/StdLib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/io.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/regex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/typeconv.cpp
)

set(STDLIB_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/core/file_properties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/core/system.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Stdlib/StdLib.hpp
)

add_library(PhasorRuntime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS} ${STDLIB_SOURCES} ${STDLIB_HEADERS})