cmake_minimum_required(VERSION 3.10)
project(Phasor VERSION 3.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
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
    set(PLUGIN_INSTALL_DIR "opt/Phasor/plugins")
endif()

include_directories(include src ${CMAKE_CURRENT_BINARY_DIR}/include)

option(ASSEMBLY "Enable assembly for core code" ON)

if(WIN32 AND ASSEMBLY)
    enable_language(RC)
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/version.h"
    @ONLY
)

# Configure
include(cmake/CompilerFlags.cmake)
include(cmake/AssemblyConfig.cmake)

# Build
add_subdirectory(src)

# Install
include(cmake/InstallConfig.cmake)