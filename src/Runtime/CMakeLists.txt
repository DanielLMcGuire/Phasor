set(RUNTIME_SOURCES
    VM/Native.cpp
    VM/Operations.cpp
    VM/Register.cpp
    VM/Stack.cpp
    VM/Utility.cpp
    VM/Variables.cpp
    FFI/ffi.cpp
    FFI/api.cpp
    VM/core/IO/IO.c
)

if(ASSEMBLY)
    list(APPEND RUNTIME_SOURCES ${ARITHMETIC_ASM} ${LOGICAL_ASM})
else()
    list(APPEND RUNTIME_SOURCES
        VM/core/arithmetic/crt.c
        VM/core/logical/crt.c
    )
endif()

set(RUNTIME_HEADERS
    Value.hpp
    VM/VM.hpp
    VM/core/core.h
    VM/core/arithmetic/arithmetic.h
    VM/core/logical/logical.h
    VM/core/IO/IO.h
    ${CMAKE_SOURCE_DIR}/src/sscanf.h
    FFI/ffi.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
    ${CMAKE_SOURCE_DIR}/src/AST/AST.hpp
    ${CMAKE_BINARY_DIR}/include/version.h
)

set(STDLIB_SOURCES
    Stdlib/core/file_properties.c
    Stdlib/core/system.c
    Stdlib/StdLib.cpp
    Stdlib/file.cpp
    Stdlib/io.cpp
    Stdlib/regex.cpp
    Stdlib/math.cpp
    Stdlib/string.cpp
    Stdlib/system.cpp
    Stdlib/typeconv.cpp
)

set(STDLIB_HEADERS
    Stdlib/core/file_properties.h
    Stdlib/core/system.h
    Stdlib/StdLib.hpp
)

add_library(PhasorRuntime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS} ${STDLIB_SOURCES} ${STDLIB_HEADERS})