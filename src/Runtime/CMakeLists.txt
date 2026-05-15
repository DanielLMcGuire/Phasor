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
if(USE_PCH)
target_precompile_headers(PhasorRuntime PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/VM/VM.hpp>")
endif()
add_library(phasor_native_runtime_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/NativeRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Shared/NativeRuntime.hpp
)

find_package(LLVM REQUIRED CONFIG)

target_include_directories(PhasorRuntime PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(PhasorRuntime PRIVATE ${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS
    core
    orcjit
    native
)

target_link_libraries(PhasorRuntime PRIVATE ${LLVM_LIBS})
target_include_directories(PhasorRuntime PUBLIC ${LLVM_INCLUDE_DIRS})

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