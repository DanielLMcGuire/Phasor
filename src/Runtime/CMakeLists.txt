set(RUNTIME_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Native.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Operations.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Register.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Stack.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Utility.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/Variables.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/ffi.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/api.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/IO/IO.c
)

if(ASSEMBLY)
    list(APPEND RUNTIME_SOURCES ${ARITHMETIC_ASM} ${LOGICAL_ASM})
else()
    list(APPEND RUNTIME_SOURCES
        ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/arithmetic/crt.c
        ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/logical/crt.c
    )
endif()

set(RUNTIME_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Runtime/Value.hpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/VM.hpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/core.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/arithmetic/arithmetic.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/logical/logical.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/VM/core/IO/IO.h
    ${CMAKE_SOURCE_DIR}/src/sscanf.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/FFI/ffi.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
    ${CMAKE_SOURCE_DIR}/src/AST/AST.hpp
    ${CMAKE_BINARY_DIR}/version.h
)

set(STDLIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/file_properties.c
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/system.c
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/StdLib.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/file.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/io.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/regex.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/math.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/string.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/system.cpp
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/typeconv.cpp
)

set(STDLIB_HEADERS
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/file_properties.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/core/system.h
    ${CMAKE_SOURCE_DIR}/src/Runtime/Stdlib/StdLib.hpp
)

add_library(PhasorRuntime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS} ${STDLIB_SOURCES} ${STDLIB_HEADERS})