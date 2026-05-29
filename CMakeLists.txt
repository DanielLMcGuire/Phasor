cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0091 NEW)
project(Phasor VERSION 3.3.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Default install prefix" FORCE)
endif()

if(WIN32)
    set(PLUGIN_INSTALL_DIR "bin/plugins")
elseif(APPLE)
    set(PLUGIN_INSTALL_DIR "Library/Application Support/org.Phasor.Phasor/plugins")
else()
    set(PLUGIN_INSTALL_DIR "usr/lib/phasor/plugins")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

option(ASSEMBLY "Enable assembly for core code" ON)
option(IS_TRACING "Enable VM tracing to stdout" OFF)
option(IS_TIMING "Time how long a program takes to run" OFF)
option(IS_TRACING_STACK "Enable VM stack tracing to stdout" OFF)
option(IS_TRACING_ALLOCATIONS "Enable tracking of memory allocations and leaks" OFF)
option(IS_SANDBOXED "Enable VM sandbox" OFF)
option(EMBEDDED "Strip down to the essentials" OFF)
option(CLANG_SAN_ADDR  "Enable Clang AddressSanitizer"  OFF)
option(CLANG_SAN_LEAK  "Enable Clang LeakSanitizer" OFF)
option(CLANG_USES_LLVM_RUNTIME "Clang is configured to use compiler-rt/libunwind/libc++ instead of GCC's runtime" ON)
option(STATIC "Compile executables statically against C/CXX RT" OFF)
option(NATIVE "Use if you only plan to use on THIS machine" OFF)
option(FORCE_PROJECT_VERSION "Force project version string to lock to cmake project_version" OFF)
option(USE_PCH "Use PCH" ON)
option(PGO_A "Instrument PGO data" OFF)
option(PGO_B "Use PGO data" OFF)

if(WIN32 AND ASSEMBLY)
    enable_language(RC)
endif()

# Prebuild config
include(cmake/Preprocessor.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/AssemblyConfig.cmake)
include(cmake/Version.cmake)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/version.h"
    @ONLY
)

# Build config
add_subdirectory(src)

if(EMBEDDED)
    set(IS_SANDBOXED ON)
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(USE_PCH ON)
    if(NOT IS_XBOX AND NOT EMBEDDED)
        add_subdirectory(thirdparty)
    endif()
    include(cmake/InstallConfig.cmake)
endif()

if(USE_PCH)
    set(CMAKE_PCH ON)
endif()

if(USE_PCH)
    include(cmake/PCH.cmake)
    APPLY_GLOBAL_PCH("$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/Value.hpp>")
endif()