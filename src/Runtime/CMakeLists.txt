set(RUNTIME_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Value.hpp
    ${CMAKE_SOURCE_DIR}/src/include/sscanf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FFI/ffi.hpp
    ${CMAKE_SOURCE_DIR}/include/PhasorFFI.h
    ${CMAKE_SOURCE_DIR}/src/ISA/ISA.hpp
    ${CMAKE_SOURCE_DIR}/src/ISA/map.hpp
    ${CMAKE_BINARY_DIR}/include/version.h
)

add_subdirectory(FFI)
add_subdirectory(VM)
add_subdirectory(Stdlib)

add_library(PhasorRuntime STATIC ${VM_SOURCES} ${VM_HEADERS} ${FFI_SOURCES} ${FFI_HEADERS} ${RUNTIME_HEADERS} ${STDLIB_SOURCES} ${STDLIB_HEADERS})
add_library(Phasor::Runtime ALIAS PhasorRuntime)
if(USE_PCH)
target_precompile_headers(PhasorRuntime PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/VM/VM.hpp>")
endif()
add_library(phasor_native_runtime_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/NativeRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Library/NativeRuntime.hpp
)
target_link_libraries(PhasorRuntime PUBLIC PhasorCodegen phasor_language)

target_link_libraries(phasor_native_runtime_lib PUBLIC PhasorRuntime "$<$<PLATFORM_ID:Darwin>:-framework CoreFoundation>" "$<$<PLATFORM_ID:Darwin>:-framework Security>")