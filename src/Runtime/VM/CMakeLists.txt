set(VM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Operations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Register.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Variables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/IO/IO.c
)

if(ASSEMBLY)
    list(APPEND VM_SOURCES ${ARITHMETIC_ASM} ${LOGICAL_ASM})
else()
    list(APPEND VM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/arithmetic/crt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/core/logical/crt.c
    )
endif()

set(VM_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/VM.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/arithmetic/arithmetic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/logical/logical.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/IO/IO.h
)

set(VM_SOURCES ${VM_SOURCES} PARENT_SCOPE)
set(VM_HEADERS ${VM_HEADERS} PARENT_SCOPE)