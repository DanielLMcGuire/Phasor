cmake_minimum_required(VERSION 3.10)
project(PhasorFFI_Example CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the include directory for PhasorFFI.hpp, which is in the parent project's 'include' directory.
include_directories(../../../include)

# Build the shared library (plugin) from the source file.
# No linking is needed here because the Phasor API symbols will be
# resolved by the host application when it loads the plugin.
add_library(phasor_ffi_example SHARED src/ffi.cpp)