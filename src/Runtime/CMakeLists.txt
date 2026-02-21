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

add_library(phasor_native_runtime_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/NativeRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/NativeRuntime.hpp
)

target_link_libraries(phasor_native_runtime_lib PUBLIC PhasorRuntime PhasorCodegen phasor_language)

add_library(phasor_scripting_runtime_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/ScriptingRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Phasor/ScriptingRuntime.hpp
)

target_link_libraries(phasor_scripting_runtime_lib PUBLIC PhasorRuntime PhasorCodegen phasor_language)

add_library(phasor_binary_runtime_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/BinaryRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/BinaryRuntime.hpp
)

target_link_libraries(phasor_binary_runtime_lib PUBLIC PhasorRuntime PhasorCodegen)
    
add_library(pulsar_scripting_runtime_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/ScriptingRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pulsar/ScriptingRuntime.hpp
)

target_link_libraries(pulsar_scripting_runtime_lib PUBLIC PhasorRuntime PhasorCodegen pulsar_language)