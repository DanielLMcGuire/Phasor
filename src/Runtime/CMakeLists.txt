set(RUNTIME_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Value.hpp
    ${CMAKE_SOURCE_DIR}/src/include/sscanf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FFI/ffi.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
    ${CMAKE_SOURCE_DIR}/src/ISA/ISA.hpp
    ${CMAKE_BINARY_DIR}/include/version.h
)

add_subdirectory(FFI)
add_subdirectory(VM)
add_subdirectory(Stdlib)

add_library(PhasorRuntime STATIC ${VM_SOURCES} ${VM_HEADERS} ${FFI_SOURCES} ${FFI_HEADERS} ${RUNTIME_HEADERS} ${STDLIB_SOURCES} ${STDLIB_HEADERS})
